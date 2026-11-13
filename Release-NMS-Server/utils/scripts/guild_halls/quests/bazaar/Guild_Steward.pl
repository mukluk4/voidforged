# Guild Steward (Bazaar): the guild's Infamy and treasury, platinum donations, and the leader's hall purchase.
# Deploy to: quests/bazaar/Guild_Steward.pl   (npc_types.id 1200124). Logic: plugins/guild_halls.pl.
# While guild halls are switched off, only GMs are answered; GMs also get the on/off switch.

sub EVENT_SAY {
	my $is_gm = $client->Admin() >= 80;
	return if !plugin::GuildHallsServes($client);

	if ($is_gm && $text =~ /^guild halls (on|off)$/i) {
		my $on = lc($1) eq 'on';
		plugin::GuildHallsSetEnabled($on);
		plugin::NPCTell($on
			? "Guild halls are now open to everyone: Infamy, the treasury and the porter."
			: "Guild halls are closed again. Only GMs are served.");
		return;
	}

	if ($text =~ /hail/i) {
		if ($is_gm) {
			my $state = plugin::GuildHallsEnabled() ? "ON" : "OFF (GMs only)";
			plugin::YellowText("[GM] Guild halls are $state: ["
				. quest::silent_saylink("guild halls on", "turn on") . "] ["
				. quest::silent_saylink("guild halls off", "turn off") . "]");
		}
	}

	if (!$client->GuildID()) {
		plugin::NPCTell("I keep the ledgers of the guilds. Join one, and I will keep yours.") if $text =~ /hail/i;
		return;
	}

	if ($text =~ /hail/i) {
		my $s = plugin::GuildHallsStatus($client) or return;
		my $hall = $s->{tier} ? "hall tier $s->{tier}" : "no hall yet";
		plugin::NPCTell(sprintf("%s has %s Infamy and %s platinum in its treasury, and %s.",
			$s->{name}, plugin::GuildHallsCommify($s->{infamy}), plugin::GuildHallsCommify($s->{treasury}), $hall));
		plugin::YellowText("Hand me platinum to add it to your guild's treasury. Donations can't be taken back. ["
			. quest::silent_saylink("infamy") . "] shows your guild's recent Infamy.");
		if ($s->{next}) {
			my $line = sprintf("Hall tier %d needs %s Infamy and %s platinum.", $s->{next_tier},
				plugin::GuildHallsCommify($s->{next}{infamy}), plugin::GuildHallsCommify($s->{next}{platinum}));
			$line .= " [" . quest::silent_saylink("buy hall") . "]" if $s->{leader};
			plugin::YellowText($line);
		}
		if ($s->{tier}) {
			my ($buffs, $next) = plugin::GuildBuffInfo($client->GuildID());
			my $line = $buffs ? "The Guild Hall Mystic grants blessings tier $buffs." : "Your hall has no Mystic blessings yet.";
			if ($next) {
				$line .= sprintf(" Tier %d needs %s Infamy and %s platinum.", $buffs + 1,
					plugin::GuildHallsCommify($next->{infamy}), plugin::GuildHallsCommify($next->{platinum}));
				$line .= " [" . quest::silent_saylink("buy blessings") . "]" if $s->{leader};
			}
			plugin::YellowText($line);
		}

	} elsif ($text =~ /^infamy$/i) {
		my $s = plugin::GuildHallsStatus($client) or return;
		if (!@{ $s->{recent} }) {
			plugin::NPCTell("$s->{name} has earned no Infamy yet. Rift clears, raid portal bosses, the great raid bosses of old and members reaching max level all earn it.");
			return;
		}
		plugin::NPCTell("$s->{name}'s most recent Infamy:");
		foreach my $entry (@{ $s->{recent} }) {
			plugin::YellowText(sprintf("  %s: %s", plugin::GuildHallsCommify($entry->[0]), $entry->[1]));
		}

	} elsif ($text =~ /^buy hall$/i) {
		my $error = plugin::GuildHallBuy($client);
		plugin::NPCTell($error ne '' ? $error : "It is done. The Guild Hall Porter will take your guild there.");

	} elsif ($text =~ /^buy blessings$/i) {
		my $error = plugin::GuildBuffBuy($client);
		plugin::NPCTell($error ne '' ? $error : "It is done. The Guild Hall Mystic will bless your members.");
	}
}

sub EVENT_ITEM {
	# Only platinum is kept; items and other coins go back through the hand-in system
	if (plugin::GuildHallsServes($client) && $client->GuildID() && $platinum > 0) {
		if (plugin::check_handin(\%itemcount, platinum => $platinum)) {
			plugin::GuildTreasuryDonate($client, $platinum);
			plugin::NPCTell("Your guild's treasury thanks you.");
		}
	}
	plugin::return_items(\%itemcount);
}
