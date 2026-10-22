-- 2026-10-22: The Void was GM-only (stock PEQ min_status 255). Opened for testing; the real lock is the
-- progression gate (tabled): add 'thevoida' to %atlas in plugins/NMS_progression_utils.pl.
-- To close it again: UPDATE zone SET min_status = 255 WHERE short_name = 'thevoida' AND version = 0;
UPDATE zone SET min_status = 0 WHERE short_name = 'thevoida' AND version = 0 AND min_status = 255;
