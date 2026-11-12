-- Targeted-AE weapon procs (Starless Volley, Event Horizon, Rift Pulse Wave) hit the wielder,
-- because a player's targeted AE includes the caster when in range. Flag them NPC-only.
-- Takes effect after a server restart (spells load into shared memory at boot).
UPDATE spells_new SET pcnpc_only_flag = 2
WHERE id BETWEEN 44100 AND 44579 AND targettype = 8;
