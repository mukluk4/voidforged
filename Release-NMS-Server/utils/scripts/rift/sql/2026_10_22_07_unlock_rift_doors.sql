-- Queued DB fixes, round 6

-- #1 Rift doors were copied with their keys/lockpick requirements (e.g. Befallen's key door).
--    Unlock every door in the rift (version 1) copies; the normal zones keep their locks.
UPDATE doors SET keyitem = 0, lockpick = 0
WHERE version = 1 AND (keyitem > 0 OR lockpick > 0)
  AND zone IN ('codecay','sebilis','chardok','befallen','karnor','mistmoore','fearplane','cazicthule',
               'charasis','permafrost','sleeper','velketor','hateplaneb','akheva','griegsend');
