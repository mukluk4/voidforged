# Glamour vendors

18 `Glamour_Outfitter` vendors in the East Commons tunnel (NPC, merchant and spawn group IDs
1200100-1200117; the generator owns 1200100-1200199) sell 2,585 armor and weapon glamours at 1,000
platinum each. Merchants won't buy glamours back (`Handle_OP_ShopPlayerSell`).

| Vendors | What they sell |
|---|---|
| Plate, Chain, Leather | Every slot, helm to boots, in 8 colors that match across all slots |
| Exotic Helms ... Exotic Boots | The other armor textures, one vendor per slot, same matching colors per texture |
| Robes | The 13 robe styles |
| 1H Slashing, 1H Blunt, 1H Piercing, Two-Handed, Shields, Fist & Ranged, Unusual Weapons | One glamour per weapon model |

Left out on purpose: Hero's Forge armor; weapons with no model (`IT0`) or the dropped-item bag
(`IT63`); armor textures 5, 7, 8, 24 and 240 (not verified on the RoF2 client); tints outside the
8-color palette; duplicates of a look already sold.

## Scripts

```
# 1. Vendor lists -> review sheet (Keep column, Patterns tab)
python gen_glamour_vendors.py <eqemu_config.json> json vendors.json
python build_review_sheet.py vendors.json glamour_review.xlsx

# 2. After editing the sheet: report, then the vendor SQL along the tunnel row
python gen_glamour_vendors.py <eqemu_config.json> report --review glamour_review.xlsx
python gen_glamour_vendors.py <eqemu_config.json> row sql/<date>_glamour_vendors_ecommons.sql ecommons 1200100 "-267.30,-1849.71,2.50" "-61.78,-1806.57,2.50" 0 --review glamour_review.xlsx

# 3. Full list of every glamour in the database, Y/N with the reason
python build_final_sheet.py <eqemu_config.json> glamour_review.xlsx glamour_final.xlsx
```

The SQL clears the whole 1200100-1200199 block before inserting, so a shorter list never leaves old
vendors behind. Only the newest file in `sql/` matches the live vendors.

Apply it with the vendor zone shut down (`#zoneshutdown zone ecommons`). The server saves a zone's live NPCs on
shutdown and restores them on boot (`Zone:StateSavingOnShutdown`), so the SQL also clears that zone's saved
state; with the zone still up, its next shutdown would save the old vendors again.
