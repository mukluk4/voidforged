# Guild Hall Porter (Bazaar): sends members of a guild that owns a hall to it (their guild's private instance).
# Deploy to: quests/bazaar/Guild_Hall_Porter.pl   (npc_types.id 1200118). Logic: plugins/guild_halls.pl.
# While guild halls are switched off, only GMs are answered.

sub EVENT_SAY {
	return if !plugin::GuildHallsServes($client);

	if (!$client->GuildID()) {
		plugin::NPCTell("I carry guild members to their halls. You have no guild.") if $text =~ /hail/i;
		return;
	}

	if (!plugin::GuildHallTier($client->GuildID())) {
		plugin::NPCTell("Your guild has no hall yet. Its leader can buy one from the Guild Steward.")
			if $text =~ /hail|enter/i;
		return;
	}

	if ($text =~ /hail/i) {
		plugin::NPCTell("Your guild's hall awaits. [" . quest::silent_saylink("enter") . "]");
	} elsif ($text =~ /^enter$/i) {
		my $error = plugin::GuildHallEnter($client);
		plugin::NPCTell($error) if $error ne '';
	}
}
