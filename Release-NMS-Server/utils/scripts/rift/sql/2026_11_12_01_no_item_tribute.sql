-- Tribute is platinum only on Voidforged (2026-11-12): no item has a tribute or guild tribute value.
-- The zone also refuses item tribute in code (tribute.cpp, client_packet.cpp, nms_loot_offers.cpp), so items added
-- later with a favor value still give nothing. Item data loads through shared_memory: restart after applying.
UPDATE items SET favor = 0, guildfavor = 0 WHERE favor <> 0 OR guildfavor <> 0;
