# Guild Hall Mystic (guild hall in Highkeep, version 60): buffs a member and their pet with the blessings tier their guild bought.
# Deploy to: quests/highkeep/Guild_Hall_Mystic.pl   (npc_types.id 1200119). Logic: plugins/guild_halls.pl.
# While guild halls are switched off, only GMs are answered.

sub EVENT_SAY {
	return if !plugin::GuildHallsServes($client);
	return if !$client->GuildID();

	# Hail to be blessed; no cooldown
	return if $text !~ /hail/i;
	my $tier = plugin::GuildBuffApply($client);
	plugin::NPCTell($tier
		? "Go with your guild's blessings, tier $tier."
		: "I have no blessings to give yet. Your guild's leader can buy them from the Guild Steward in the Bazaar.");
}
