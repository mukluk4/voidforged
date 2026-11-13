# Hall Doorman (guild hall in Highkeep, version 60): the way out, back to the Bazaar beside the Guild Hall Porter.
# Deploy to: quests/highkeep/Hall_Doorman.pl   (npc_types.id 1200123). Always answers, so nobody is ever stuck.

sub EVENT_SAY {
	if ($text =~ /hail/i) {
		plugin::NPCTell("Leaving already? [" . quest::silent_saylink("return to the Bazaar") . "]");
	} elsif ($text =~ /^return to the bazaar$/i) {
		$client->MovePC(151, -135.00, -790.00, 3.75, 128.00);
	}
}
