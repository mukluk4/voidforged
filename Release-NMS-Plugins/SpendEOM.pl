
# Echo of Memory was removed (2026-10-25): services that cost Echoes now cost platinum, 1 Echo = 1,000pp.
sub SpendPlatinum {
    my ($client, $platinum) = @_;
    return 0 unless $client;
    return $client->TakeMoneyFromPP($platinum * 1000, 1);
}

sub HasPlatinum {
    my ($client, $platinum) = @_;
    return $client && $client->GetCarriedMoney() >= $platinum * 1000;
}

sub ApplyWorldWideBuff {
    my $spell_id = shift;
    my $duration = shift || 14400; # Default to 4 hours (14400 seconds)
    
    # Set/extend the global buff time via native C++ quest API
    my $expiration = quest::add_global_buff($spell_id, $duration);
    
    # Broadcast the blessing activation/extension to the world
    my $spell_name = quest::getspellname($spell_id);
    my $remaining_seconds = $expiration - time();
    
    if ($remaining_seconds > 0) {
        my $hours = int($remaining_seconds / 3600);
        my $minutes = int(($remaining_seconds % 3600) / 60);
        
        my $time_str = "";
        $time_str .= "$hours hours " if $hours > 0;
        $time_str .= "$minutes minutes" if $minutes > 0;
        $time_str =~ s/\s+$//; # Clean up trailing space if no minutes
        
        plugin::WorldAnnounce("A server-wide blessing of [$spell_name] has been activated/extended! Remaining duration: $time_str.");
    }
    
    # Force reload of global buffs across all zones
    quest::reload_global_buffs();
    
    return 1;
}

1;
