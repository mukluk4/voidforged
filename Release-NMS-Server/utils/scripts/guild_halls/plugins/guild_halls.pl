# Guild halls, phase 1: Infamy, the guild treasury and the T1 hall. See docs/guild_halls/GUILD_HALLS.md.
#
# Off until a GM turns it on at the Guild Steward in the Bazaar (data bucket guild_halls_enabled = 1).
# While off, only GM accounts earn Infamy, see guild messages, or get served by the Steward and the
# porter, so it can be tested without players seeing anything.
#
# Called from global_npc.pl (EVENT_DEATH_COMPLETE), global_player.pl (EVENT_LEVEL_UP) and
# raid_portal.pl (RaidPortalDeath). Tables: nms_guild_* (sql/2026_11_13_01_guild_halls_phase1.sql).

my $GH_ENABLED_KEY      = 'guild_halls_enabled';
my $GH_GM_STATUS        = 80;      # accounts at or above this count as GMs while the switch is off
my $GH_RIFT_GUARDIAN    = 1200001; # Rift_Guardian
my $GH_RIFT_DIVISOR     = 10;      # rift clear: tier / 10 Infamy per guild member (at least 1)
my $GH_RAID_PER_TIER    = 25;      # raid portal boss: 25 x raid tier per guild member
my $GH_RAID_VERSION     = 50;      # raid portal instances; their bosses pay through raid_portal.pl
my $GH_FIRST_KILL_MULT  = 5;       # a guild's first kill of a boss pays 5x, once per boss
my $GH_MAX_LEVEL_INFAMY = 100;     # a member reaching Character:MaxLevel, once per character
my $GH_BOSS_LOCKOUT     = 1800;    # multi-phase bosses: one award per boss per zone copy per 30 minutes

# Open-world raid bosses pay by era (nms_guild_infamy_bosses.era = zone.expansion)
my %GH_ERA_INFAMY = (0 => 10, 1 => 15, 2 => 20, 3 => 25, 4 => 30);

# Hall tiers that can be bought, and where each hall is: a private instance per guild of that zone's guild
# hall version. T1 is Highkeep's third floor (sql/2026_11_13_03_highkeep_hall.sql); T2-T5 are not built yet.
my %GH_HALL_TIERS = (
	1 => { infamy => 1000, platinum => 250000, name => 'the High Keep hall',
		zone => 'highkeep', zone_id => 6, version => 60, x => -163, y => 80, z => 65.63, h => 0 },
);
my $GH_HALL_SECONDS = 90 * 86400;   # a hall instance lives 90 days, renewed every time a member enters

# No levitation (owner 2026-11-13: no Spirit of Eagle). Guild Hall Mystic buff tiers, bought in order at the Steward (needs a hall). Hailing the Mystic applies every
# spell of the guild's tier to the member and their pet, at normal duration, with normal stacking rules.
my %GH_BUFF_TIERS = (
	1 => { infamy => 2500,  platinum => 100000,
		spells => [3692, 168, 174, 172, 356, 278] },          # Temperance, Talisman of Altuna, Clarity, Swift Like the Wind, Shield of Thorns, Spirit of Wolf
	2 => { infamy => 7500,  platinum => 250000,
		spells => [1447, 1585, 1693, 1709, 1560, 278] },      # Aegolism, Talisman of Kragg, Clarity II, Wondrous Rapidity, Shield of Blades, Spirit of Wolf
	3 => { infamy => 15000, platinum => 500000,
		spells => [3467, 1432, 1410, 1710, 3198, 278] },      # Virtue, Focus of Spirit, Gift of Brilliance, Visions of Grandeur, Flameshield of Ro, Spirit of Wolf
	4 => { infamy => 30000, platinum => 1000000,
		spells => [3467, 3235, 3360, 3240, 3486, 3234, 278] }, # Virtue, Focus of Soul, Voice of Quellious, Speed of Vallon, Maelstrom of Ro, Protection of the Nine, Spirit of Wolf
	5 => { infamy => 50000, platinum => 2000000,
		spells => [5257, 3235, 5513, 5507, 5358, 3463, 3417, 278] }, # Conviction, Focus of Soul, Clairvoyance, Speed of Salik, Nettle Shield, Ferocity, Spirit of the Predator, Spirit of Wolf
);
my $GH_BUFF_UNLOCK = 'buffs';

my %gh_bosses;          # npc id => era, loaded once per zone process
my $gh_bosses_loaded = 0;

sub GuildHallsEnabled {
	return quest::get_data($GH_ENABLED_KEY) eq '1';
}

sub GuildHallsSetEnabled {
	my $on = shift;
	quest::set_data($GH_ENABLED_KEY, $on ? '1' : '0');
}

# perlbind only accepts an integer argument from an integer scalar
sub GuildHallsInt {
	return sprintf("%d", shift) + 0;
}

# A message to the guild, everywhere in the world. While the switch is off only GMs see it.
sub GuildHallsTell {
	my ($guild_id, $message) = @_;
	my $min_status = plugin::GuildHallsEnabled() ? 0 : $GH_GM_STATUS;
	quest::gmsay($message, 15, 1, plugin::GuildHallsInt($guild_id), plugin::GuildHallsInt($min_status));
}

# Whether this client may use guild hall features right now
sub GuildHallsServes {
	my $client = shift;
	return plugin::GuildHallsEnabled() || $client->Admin() >= $GH_GM_STATUS;
}

sub GuildHallsDB {
	return plugin::LoadMysqlServer();
}

# The guild's row: (infamy, treasury, hall_tier), zeros when it has none yet
sub GuildHallsGetGuild {
	my ($dbh, $guild_id) = @_;
	my $row = $dbh->selectrow_arrayref(
		"SELECT infamy, treasury, hall_tier FROM nms_guild_halls WHERE guild_id = ?", undef, $guild_id);
	return $row ? @$row : (0, 0, 0);
}

sub GuildHallsGuildName {
	my ($dbh, $guild_id) = @_;
	my $row = $dbh->selectrow_arrayref("SELECT name FROM guilds WHERE id = ?", undef, $guild_id);
	return $row ? $row->[0] : "your guild";
}

sub GuildHallsIsLeader {
	my ($dbh, $guild_id, $char_id) = @_;
	my $row = $dbh->selectrow_arrayref("SELECT leader FROM guilds WHERE id = ?", undef, $guild_id);
	return $row && $row->[0] == $char_id;
}

sub GuildHallsCommify {
	my $n = reverse shift;
	$n =~ s/(\d{3})(?=\d)/$1,/g;
	return scalar reverse $n;
}

# Pays $per_member Infamy for each credited character to that character's guild.
# $first_key: a boss key; the guild's first kill of it pays $GH_FIRST_KILL_MULT times as much.
sub GuildInfamyAward {
	my ($char_ids, $per_member, $source, $first_key, $label) = @_;
	return if !$char_ids || !@$char_ids || $per_member <= 0;

	my $dbh = plugin::GuildHallsDB() or return;
	my $enabled = plugin::GuildHallsEnabled();

	my $marks = join(',', ('?') x @$char_ids);
	my $sth = $dbh->prepare(
		"SELECT gm.char_id, gm.guild_id, a.status FROM guild_members gm
		 JOIN character_data c ON c.id = gm.char_id JOIN account a ON a.id = c.account_id
		 WHERE gm.char_id IN ($marks)");
	$sth->execute(@$char_ids);

	my %members;
	while (my ($char_id, $guild_id, $status) = $sth->fetchrow_array()) {
		next if !$guild_id;
		next if !$enabled && $status < $GH_GM_STATUS;
		$members{$guild_id}++;
	}

	foreach my $guild_id (keys %members) {
		my $amount = $per_member * $members{$guild_id};
		my $first  = 0;
		if ($first_key) {
			my $rows = $dbh->do(
				"INSERT IGNORE INTO nms_guild_first_kills (guild_id, boss_key) VALUES (?, ?)", undef, $guild_id, $first_key);
			if ($rows == 1) {
				$amount *= $GH_FIRST_KILL_MULT;
				$first = 1;
			}
		}

		$dbh->do(
			"INSERT INTO nms_guild_halls (guild_id, infamy) VALUES (?, ?)
			 ON DUPLICATE KEY UPDATE infamy = infamy + VALUES(infamy)", undef, $guild_id, $amount);
		$dbh->do(
			"INSERT INTO nms_guild_infamy_log (guild_id, amount, members, source, detail) VALUES (?, ?, ?, ?, ?)",
			undef, $guild_id, $amount, $members{$guild_id}, $source, substr($label, 0, 128));

		my ($infamy) = plugin::GuildHallsGetGuild($dbh, $guild_id);
		my $who = $members{$guild_id} == 1 ? "1 member" : "$members{$guild_id} members";
		plugin::GuildHallsTell($guild_id, sprintf("Your guild earns %s Infamy: %s, %s%s. Infamy: %s.",
			plugin::GuildHallsCommify($amount), $label, $who, ($first ? ", first kill x$GH_FIRST_KILL_MULT" : ""),
			plugin::GuildHallsCommify($infamy)));
	}

	$dbh->disconnect();
}

# global_npc.pl EVENT_DEATH_COMPLETE: rift clears and open-world raid bosses
sub GuildInfamyNPCDeath {
	my ($npc, $killer_id) = @_;
	my $npc_id      = $npc->GetNPCTypeID();
	my $instance_id = plugin::val('$instanceid') || 0;

	# Rift Guardian: rift_core.lua's rift.complete marks the instance cleared before this runs
	if ($npc_id == $GH_RIFT_GUARDIAN) {
		return if !$instance_id;
		my $tier = quest::get_data("gr_cleared_$instance_id");
		return if $tier eq '' || quest::get_data("gh_rift_paid_$instance_id") ne '';
		quest::set_data("gh_rift_paid_$instance_id", "1", "3600");

		my $per = int($tier / $GH_RIFT_DIVISOR);
		$per = 1 if $per < 1;
		my @char_ids = map { $_->CharacterID() } plugin::val('$entity_list')->GetClientList();
		plugin::GuildInfamyAward(\@char_ids, $per, 'rift', '', "Tier $tier rift cleared");
		return;
	}

	return if !plugin::GuildInfamyIsBoss($npc_id);
	# Raid portal copies pay through raid_portal.pl; rift instances have their own reward
	return if plugin::val('$instanceversion') == $GH_RAID_VERSION;
	return if $instance_id && quest::get_data("gr_tier_$instance_id") ne '';

	my $key = $npc->GetCleanName();
	$key =~ s/^#//;
	$key =~ s/_+$//;
	$key =~ s/[^A-Za-z0-9]+/_/g;
	my $lock = "gh_boss_" . lc($key) . "_" . plugin::val('$zoneid') . "_$instance_id";
	return if quest::get_data($lock) ne '';
	quest::set_data($lock, "1", "$GH_BOSS_LOCKOUT");

	my @char_ids = plugin::GuildInfamyKillers($killer_id);
	my $era = $gh_bosses{$npc_id};
	plugin::GuildInfamyAward(\@char_ids, $GH_ERA_INFAMY{$era} || 10, 'boss', "boss:" . lc($key), $npc->GetCleanName());
}

sub GuildInfamyIsBoss {
	my $npc_id = shift;
	if (!$gh_bosses_loaded) {
		$gh_bosses_loaded = 1;
		my $dbh = plugin::GuildHallsDB() or return 0;
		my $rows = $dbh->selectall_arrayref("SELECT npc_id, era FROM nms_guild_infamy_bosses");
		$gh_bosses{$_->[0]} = $_->[1] foreach @{ $rows || [] };
		$dbh->disconnect();
	}
	return exists $gh_bosses{$npc_id};
}

# The killer (a pet's owner counts) and their raid or group members in this zone
sub GuildInfamyKillers {
	my $killer_id   = shift;
	my $entity_list = plugin::val('$entity_list');
	my $killer      = $killer_id ? $entity_list->GetMobByID($killer_id) : undef;
	return () if !$killer;

	if (!$killer->IsClient() && $killer->GetOwner()) {
		$killer = $killer->GetOwner();
	}
	return () if !$killer->IsClient();
	$killer = $killer->CastToClient();

	my %seen = ($killer->CharacterID() => 1);
	my $raid = $killer->GetRaid();
	if ($raid) {
		for my $i (0 .. 71) {
			my $member = $raid->GetMember($i);
			$seen{$member->CharacterID()} = 1 if $member && $member->IsClient();
		}
	} elsif (my $group = $killer->GetGroup()) {
		for my $i (0 .. 5) {
			my $member = $group->GetMember($i);
			$seen{$member->CastToClient()->CharacterID()} = 1 if $member && $member->IsClient();
		}
	}
	return keys %seen;
}

# raid_portal.pl RaidPortalDeath, once per paid raid
sub GuildInfamyRaid {
	my ($char_ids, $tier, $boss_id, $boss_name) = @_;
	plugin::GuildInfamyAward($char_ids, $GH_RAID_PER_TIER * $tier, 'raid', "raid:$boss_id",
		"$boss_name, raid tier $tier");
}

# global_player.pl EVENT_LEVEL_UP: once per character, at the server's max level
sub GuildInfamyMaxLevel {
	my $client = shift;
	return if $client->GetLevel() < quest::get_rule("Character:MaxLevel");
	return if !$client->GuildID() || !plugin::GuildHallsServes($client);

	my $dbh = plugin::GuildHallsDB() or return;
	my $rows = $dbh->do("INSERT IGNORE INTO nms_guild_max_level (char_id, guild_id) VALUES (?, ?)",
		undef, $client->CharacterID(), $client->GuildID());
	$dbh->disconnect();
	return if $rows != 1;

	plugin::GuildInfamyAward([$client->CharacterID()], $GH_MAX_LEVEL_INFAMY, 'level', '',
		$client->GetCleanName() . " reached level " . $client->GetLevel());
}

# Guild Steward: platinum handed in goes to the treasury. Returns the amount kept.
sub GuildTreasuryDonate {
	my ($client, $platinum) = @_;
	return 0 if $platinum <= 0;

	my $dbh = plugin::GuildHallsDB() or return 0;
	my $guild_id = $client->GuildID();
	$dbh->do(
		"INSERT INTO nms_guild_halls (guild_id, treasury) VALUES (?, ?)
		 ON DUPLICATE KEY UPDATE treasury = treasury + VALUES(treasury)", undef, $guild_id, $platinum);
	$dbh->do("INSERT INTO nms_guild_treasury_log (guild_id, char_id, amount, reason) VALUES (?, ?, ?, 'donation')",
		undef, $guild_id, $client->CharacterID(), $platinum);
	my (undef, $treasury) = plugin::GuildHallsGetGuild($dbh, $guild_id);
	$dbh->disconnect();

	plugin::GuildHallsTell($guild_id, sprintf("%s donates %s platinum to the guild treasury. Treasury: %s platinum.",
		$client->GetCleanName(), plugin::GuildHallsCommify($platinum), plugin::GuildHallsCommify($treasury)));
	return $platinum;
}

# Guild Steward: the leader buys the next hall tier. Returns an error message, or '' on success.
sub GuildHallBuy {
	my $client   = shift;
	my $guild_id = $client->GuildID();
	my $dbh = plugin::GuildHallsDB() or return "The ledgers are closed. Try again later.";

	if (!plugin::GuildHallsIsLeader($dbh, $guild_id, $client->CharacterID())) {
		$dbh->disconnect();
		return "Only your guild's leader may buy a hall.";
	}

	my ($infamy, $treasury, $tier) = plugin::GuildHallsGetGuild($dbh, $guild_id);
	my $next = $tier + 1;
	my $cost = $GH_HALL_TIERS{$next};
	if (!$cost) {
		$dbh->disconnect();
		return "There is no larger hall to buy yet.";
	}
	if ($infamy < $cost->{infamy} || $treasury < $cost->{platinum}) {
		$dbh->disconnect();
		return sprintf("Hall tier %d needs %s Infamy and %s platinum in the treasury. You have %s Infamy and %s platinum.",
			$next, plugin::GuildHallsCommify($cost->{infamy}), plugin::GuildHallsCommify($cost->{platinum}),
			plugin::GuildHallsCommify($infamy), plugin::GuildHallsCommify($treasury));
	}

	# Guarded update, so two purchases can't both spend the same platinum
	my $rows = $dbh->do(
		"UPDATE nms_guild_halls SET treasury = treasury - ?, hall_tier = ?, hall_bought_at = NOW()
		 WHERE guild_id = ? AND hall_tier = ? AND infamy >= ? AND treasury >= ?",
		undef, $cost->{platinum}, $next, $guild_id, $tier, $cost->{infamy}, $cost->{platinum});
	if ($rows != 1) {
		$dbh->disconnect();
		return "The ledgers changed while you were speaking. Ask again.";
	}
	$dbh->do("INSERT INTO nms_guild_treasury_log (guild_id, char_id, amount, reason) VALUES (?, ?, ?, ?)",
		undef, $guild_id, $client->CharacterID(), -$cost->{platinum}, "hall tier $next");
	$dbh->disconnect();

	plugin::GuildHallsTell($guild_id, sprintf("%s buys %s (hall tier %d) for %s platinum. The Guild Hall Porter in the Bazaar will take you there.",
		$client->GetCleanName(), $cost->{name}, $next, plugin::GuildHallsCommify($cost->{platinum})));
	return '';
}

# The level a guild owns of an unlock (amenity), 0 for none
sub GuildHallsUnlockLevel {
	my ($dbh, $guild_id, $unlock_id) = @_;
	my $row = $dbh->selectrow_arrayref(
		"SELECT level FROM nms_guild_unlocks WHERE guild_id = ? AND unlock_id = ?", undef, $guild_id, $unlock_id);
	return $row ? $row->[0] : 0;
}

# Guild Steward: the leader buys the next Mystic buff tier. Returns an error message, or '' on success.
sub GuildBuffBuy {
	my $client   = shift;
	my $guild_id = $client->GuildID();
	my $dbh = plugin::GuildHallsDB() or return "The ledgers are closed. Try again later.";

	if (!plugin::GuildHallsIsLeader($dbh, $guild_id, $client->CharacterID())) {
		$dbh->disconnect();
		return "Only your guild's leader may buy the Mystic's blessings.";
	}

	my ($infamy, $treasury, $hall) = plugin::GuildHallsGetGuild($dbh, $guild_id);
	if (!$hall) {
		$dbh->disconnect();
		return "The Mystic serves in a guild hall. Buy a hall first.";
	}

	my $level = plugin::GuildHallsUnlockLevel($dbh, $guild_id, $GH_BUFF_UNLOCK);
	my $next  = $level + 1;
	my $cost  = $GH_BUFF_TIERS{$next};
	if (!$cost) {
		$dbh->disconnect();
		return "Your guild already has the Mystic's highest blessings.";
	}
	if ($infamy < $cost->{infamy} || $treasury < $cost->{platinum}) {
		$dbh->disconnect();
		return sprintf("Blessings tier %d needs %s Infamy and %s platinum in the treasury. You have %s Infamy and %s platinum.",
			$next, plugin::GuildHallsCommify($cost->{infamy}), plugin::GuildHallsCommify($cost->{platinum}),
			plugin::GuildHallsCommify($infamy), plugin::GuildHallsCommify($treasury));
	}

	# Pay and record in one transaction; the guarded update stops two purchases spending the same platinum
	$dbh->begin_work();
	my $paid = $dbh->do(
		"UPDATE nms_guild_halls SET treasury = treasury - ? WHERE guild_id = ? AND infamy >= ? AND treasury >= ?",
		undef, $cost->{platinum}, $guild_id, $cost->{infamy}, $cost->{platinum});
	my $set = $paid == 1 ? $dbh->do(
		"INSERT INTO nms_guild_unlocks (guild_id, unlock_id, level) VALUES (?, ?, ?)
		 ON DUPLICATE KEY UPDATE level = IF(level = ? - 1, VALUES(level), level)",
		undef, $guild_id, $GH_BUFF_UNLOCK, $next, $next) : 0;
	# MariaDB reports 2 rows for an update through ON DUPLICATE KEY, 1 for a new row, 0 if level didn't match
	if ($paid != 1 || ($set != 1 && $set != 2)) {
		$dbh->rollback();
		$dbh->disconnect();
		return "The ledgers changed while you were speaking. Ask again.";
	}
	$dbh->do("INSERT INTO nms_guild_treasury_log (guild_id, char_id, amount, reason) VALUES (?, ?, ?, ?)",
		undef, $guild_id, $client->CharacterID(), -$cost->{platinum}, "mystic blessings tier $next");
	$dbh->commit();
	$dbh->disconnect();

	plugin::GuildHallsTell($guild_id, sprintf("%s buys the Guild Hall Mystic's blessings, tier %d, for %s platinum.",
		$client->GetCleanName(), $next, plugin::GuildHallsCommify($cost->{platinum})));
	return '';
}

# Guild Hall Mystic: buff the member and their pet with the guild's tier. Returns the tier used (0 = none).
sub GuildBuffApply {
	my $client = shift;
	my $dbh = plugin::GuildHallsDB() or return 0;
	my $level = plugin::GuildHallsUnlockLevel($dbh, $client->GuildID(), $GH_BUFF_UNLOCK);
	$dbh->disconnect();
	my $tier = $GH_BUFF_TIERS{$level} or return 0;

	my @targets = ($client);
	my $pet = $client->GetPet();
	push @targets, $pet if $pet;
	foreach my $target (@targets) {
		$target->ApplySpellBuff(plugin::GuildHallsInt($_)) foreach @{ $tier->{spells} };
	}
	return $level;
}

# The guild's Mystic tier and the next tier's cost (undef at the top)
sub GuildBuffInfo {
	my $guild_id = shift;
	my $dbh = plugin::GuildHallsDB() or return (0, undef);
	my $level = plugin::GuildHallsUnlockLevel($dbh, $guild_id, $GH_BUFF_UNLOCK);
	$dbh->disconnect();
	return ($level, $GH_BUFF_TIERS{$level + 1});
}

# Guild Hall Porter: send a member to their guild's hall instance, creating it if it's gone.
# Returns an error message, or '' once the member is on their way.
sub GuildHallEnter {
	my $client   = shift;
	my $guild_id = $client->GuildID();
	my $tier     = plugin::GuildHallTier($guild_id);
	my $hall;
	for (my $t = $tier; $t >= 1 && !$hall; $t--) {
		$hall = $GH_HALL_TIERS{$t};
	}
	return "Your guild has no hall yet. Its leader can buy one from the Guild Steward." if !$hall;

	# One instance per guild and hall zone; a new one if it expired or the guild moved up a tier
	my $key = "gh_hall_instance_${guild_id}_$hall->{zone}_$hall->{version}";
	my $instance_id = quest::get_data($key) || 0;
	if (!$instance_id || quest::GetInstanceTimerByID(plugin::GuildHallsInt($instance_id)) <= 0) {
		$instance_id = quest::CreateInstance($hall->{zone}, plugin::GuildHallsInt($hall->{version}),
			plugin::GuildHallsInt($GH_HALL_SECONDS));
		return "The way to your hall will not open. Try again." if !$instance_id || $instance_id <= 0;
		quest::set_data($key, "$instance_id");
	} else {
		quest::UpdateInstanceTimer(plugin::GuildHallsInt($instance_id), plugin::GuildHallsInt($GH_HALL_SECONDS));
	}

	$instance_id = plugin::GuildHallsInt($instance_id);
	quest::AssignToInstanceByCharID($instance_id, plugin::GuildHallsInt($client->CharacterID()));
	$client->MovePCInstance(plugin::GuildHallsInt($hall->{zone_id}), $instance_id, $hall->{x}, $hall->{y}, $hall->{z}, $hall->{h});
	return '';
}

# The guild's hall tier, 0 for none
sub GuildHallTier {
	my $guild_id = shift;
	return 0 if !$guild_id;
	my $dbh = plugin::GuildHallsDB() or return 0;
	my (undef, undef, $tier) = plugin::GuildHallsGetGuild($dbh, $guild_id);
	$dbh->disconnect();
	return $tier;
}

# Steward's status lines for the hailer's guild
sub GuildHallsStatus {
	my $client   = shift;
	my $guild_id = $client->GuildID();
	my $dbh = plugin::GuildHallsDB() or return ();
	my ($infamy, $treasury, $tier) = plugin::GuildHallsGetGuild($dbh, $guild_id);
	my $name   = plugin::GuildHallsGuildName($dbh, $guild_id);
	my $leader = plugin::GuildHallsIsLeader($dbh, $guild_id, $client->CharacterID());
	my $recent = $dbh->selectall_arrayref(
		"SELECT amount, detail FROM nms_guild_infamy_log WHERE guild_id = ? ORDER BY id DESC LIMIT 5", undef, $guild_id);
	$dbh->disconnect();

	my $next = $GH_HALL_TIERS{$tier + 1};
	return {
		name     => $name,
		infamy   => $infamy,
		treasury => $treasury,
		tier     => $tier,
		leader   => $leader,
		next     => $next,
		next_tier => $tier + 1,
		recent   => $recent || [],
	};
}
