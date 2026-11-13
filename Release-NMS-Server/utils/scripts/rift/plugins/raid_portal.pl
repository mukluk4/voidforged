# Raid portal scaling and payout, called from global_npc.pl for every NPC in a version 50 instance.
# The settings match rift_config.lua at the raid tier's solo tier. See docs/raids/RAID_PORTAL.md.

my $RAID_CONTROLLER = 1200070;
my $RAID_EXIT       = 1200009;  # Rift_Portal: hail to return to The Void
my $RAID_CURRENCY   = 3;        # Chronal Prisms
my $RAID_VERSION    = 50;
my $RAID_EXPIRES    = "7200";

# Bosses fought together: the portal boss and its partners must all be dead before the raid is paid.
# Vallon Zek calls Tallon Zek in at 50% (quests/potimeb/v50).
my %RAID_PARTNERS = (
	223078 => [223077],  # Vallon Zek: Tallon Zek
);

sub RaidPortalInfo {
	my $instance_id = shift;
	return () if !$instance_id;
	my $raw = quest::get_data("raid_instance_$instance_id");
	return ($raw =~ /^(\d+),(\d+),(\d+),(\d+)$/) ? ($1, $2, $3, $4) : ();
}

sub RaidPortalByTier {
	my ($base, $per_tier, $max, $solo_tier) = @_;
	my $value = int($base + $per_tier * $solo_tier);
	return plugin::RaidPortalInt($value > $max ? $max : $value);
}

# perlbind only accepts an integer argument from an integer scalar, and int() can leave a float
sub RaidPortalInt {
	return sprintf("%d", shift) + 0;
}

sub RaidPortalSpawn {
	my $npc = shift;
	return if plugin::val('$instanceversion') != $RAID_VERSION;

	my ($tier, $size, $boss_id, $spellscale) = plugin::RaidPortalInfo(plugin::val('$instanceid'));
	return if !$tier;

	my $npc_id = $npc->GetNPCTypeID();
	return if $npc_id == $RAID_CONTROLLER || $npc_id == $RAID_EXIT;

	# The solo Rift Guardian at tier x 100, then raised by raid size
	my $solo_tier = $tier * 100;
	my $steps     = $solo_tier - 1;
	my $size_dmg  = 1 + 0.05 * ($size - 1);
	$size_dmg = 1.5 if $size_dmg > 1.5;

	my $hp  = int(34500 * (1 + 0.25 * $steps) * $size);
	my $min = int(92 * (1 + 0.10 * $steps) * $size_dmg);
	my $max = int(320 * (1 + 0.10 * $steps) * $size_dmg);
	my $ac  = int(344 * (1 + 0.03 * $steps));

	# Adds and helpers are set against the boss
	if ($npc_id != $boss_id) {
		$hp  = int($hp * 0.02);
		$min = int($min * 0.5);
		$max = int($max * 0.5);
	}

	$npc->ClearItemList();
	$npc->RemoveCash();

	my $resist_add = plugin::RaidPortalByTier(0, 0.25, 100, $solo_tier);
	$npc->ModifyNPCStat("level", "70");
	$npc->ModifyNPCStat("max_hp", "$hp");
	$npc->ModifyNPCStat("min_hit", "$min");
	$npc->ModifyNPCStat("max_hit", "$max");
	$npc->ModifyNPCStat("ac", "$ac");
	$npc->ModifyNPCStat("spellscale", "$spellscale");
	$npc->ModifyNPCStat("mr", "" . ($npc->GetMR() + $resist_add));
	$npc->ModifyNPCStat("fr", "" . ($npc->GetFR() + $resist_add));
	$npc->ModifyNPCStat("cr", "" . ($npc->GetCR() + $resist_add));
	$npc->ModifyNPCStat("pr", "" . ($npc->GetPR() + $resist_add));
	$npc->ModifyNPCStat("dr", "" . ($npc->GetDR() + $resist_add));
	$npc->ModifyNPCStat("heroic_strikethrough", "" . plugin::RaidPortalByTier(0, 0.05, 25, $solo_tier));

	$npc->SetSpecialAbility(43, 1);  # Casting Resist Difficulty
	$npc->SetSpecialAbilityParam(43, 0, 0 - plugin::RaidPortalByTier(400, 2.8, 1200, $solo_tier));
	$npc->SetSpecialAbility(44, 1);  # Counter Avoid Damage
	$npc->SetSpecialAbilityParam(44, 0, plugin::RaidPortalByTier(0, 0.1, 20, $solo_tier));

	$npc->Heal();
}

# Only the portal boss pays, once, to every member assigned when the instance was created.
# A member who isn't in the zone collects on their next zone-in (RaidPortalClaim).
sub RaidPortalDeath {
	my ($npc, $x, $y, $z) = @_;
	return if plugin::val('$instanceversion') != $RAID_VERSION;

	my $instance_id = plugin::RaidPortalInt(plugin::val('$instanceid'));
	my ($tier, $size, $boss_id, $spellscale) = plugin::RaidPortalInfo($instance_id);
	return if !$tier;

	# The portal boss or one of its partners; pay only once none of the others is still up
	my $dead_id = $npc->GetNPCTypeID();
	my @group   = ($boss_id, @{ $RAID_PARTNERS{$boss_id} || [] });
	return if !grep { $_ == $dead_id } @group;
	my $entity_list = plugin::val('$entity_list');
	foreach my $id (@group) {
		next if $id == $dead_id;
		return if $entity_list->IsMobSpawnedByNpcTypeID($id);
	}

	return if quest::get_data("raid_paid_$instance_id") ne '';
	quest::set_data("raid_paid_$instance_id", "1", $RAID_EXPIRES);

	my @char_ids = (quest::GetCharactersInInstance($instance_id) =~ /\((\d+)\)/g);

	# The whole server hears about it, in yellow. The number is the raid tier's solo tier (tier x 100).
	my $players = scalar(@char_ids);
	quest::worldwidemessage(15, $npc->GetCleanName() . " Rift (" . ($tier * 100) . ") has been defeated by $players "
		. ($players == 1 ? "player" : "players"));

	my $amount      = 1 << $tier;
	foreach my $char_id (@char_ids) {
		quest::delete_data("raid_active_$char_id");

		my $member = $entity_list->GetClientByCharID($char_id + 0);
		if ($member) {
			$member->AddAlternateCurrencyValue($RAID_CURRENCY, $amount);
			$member->Message(15, "You receive $amount Chronal Prisms. Hail the portal to return to The Void.");
			plugin::RaidPortalRecordClear($member, $tier);
		} else {
			my $pending = quest::get_data("raid_fragments_pending_$char_id") || 0;
			quest::set_data("raid_fragments_pending_$char_id", "" . ($pending + $amount));
			my $cleared = quest::get_data("raid_cleared_pending_$char_id") || 0;
			quest::set_data("raid_cleared_pending_$char_id", "$tier") if $tier > $cleared;
		}
	}

	quest::spawn2($RAID_EXIT, 0, 0, $x + 5, $y + 5, $z, $npc->GetHeading());
}

# Character-scoped raid_cleared (highest raid tier cleared) unlocks that tier at the Prism_Augmenter
sub RaidPortalRecordClear {
	my ($client, $tier) = @_;
	my $cleared = $client->GetBucket("raid_cleared") || 0;
	return if $tier <= $cleared;
	$client->SetBucket("raid_cleared", "$tier");
	$client->Message(15, "Raid tier $tier cleared: the Prism Augmenter in The Void now offers tier $tier augments.");
}

sub RaidPortalClaim {
	my $client  = shift;
	my $char_id = $client->CharacterID();

	my $tier = quest::get_data("raid_cleared_pending_$char_id");
	if ($tier) {
		quest::delete_data("raid_cleared_pending_$char_id");
		plugin::RaidPortalRecordClear($client, $tier + 0);
	}

	my $key    = "raid_fragments_pending_$char_id";
	my $amount = quest::get_data($key);
	return if !$amount;
	$amount += 0;

	quest::delete_data($key);
	$client->AddAlternateCurrencyValue($RAID_CURRENCY, $amount);
	$client->Message(15, "You receive $amount Chronal Prisms from a raid you helped finish.");
}

return 1;
