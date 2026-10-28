# VoidPatcher

Players get one file, `VoidPatcher.exe`. They drop it into their EverQuest folder, or into an empty folder
for a fresh install, and run it. It downloads whatever differs from the Voidforged client, then offers Play.

- Works on a stock RoF2 client, an Imperium client, or an empty folder (full ~10 GB download).
- Only files that differ are downloaded. Each file is checked against its md5 before it replaces anything.
- Player settings (`eqclient.ini` and similar) are installed only when missing. Character UI files, logs,
  screenshots and `NMSLoot/` are never touched.
- The patcher updates itself first when a newer one is published.
- It runs on .NET Framework 4.8, which ships with Windows 10 and 11, so there is nothing else to install.
- Everything it does is written to `VoidPatcher.log` next to the exe. Ask players for that file when
  something goes wrong.

## Layout

| Path | What |
|---|---|
| `VoidPatcher/` | The patcher (C#, WinForms) |
| `build_manifest.py` | Builds `manifest.txt` and the object store from a tested client folder |
| `patch_rules.txt` | What gets shipped, excluded, or installed only when missing |

## How hosting works

The host needs plain static files and nothing else:

```
manifest.txt                 the file list (version, md5, size, path)
objects/<ab>/<abcdef...>     every file, stored under its md5
```

Files are stored by content, so a changed file always gets a new URL and no CDN can serve a stale
copy. Unchanged files are never uploaded twice. The patcher fetches `manifest.txt` with a cache-busting
query string.

The patch server URL is built into the exe (`DefaultUrl` in `Program.cs`). A `VoidPatcher.ini` next to the
exe can override it, which is handy for testing:

```
url=http://127.0.0.1:8088/
```

## Releasing a patch

1. Update and test the reference client, the one client folder every release is built from.
2. Build the publish folder. Re-runs only hash and copy the files that changed:

   ```
   python build_manifest.py --client <reference client> --out <publish folder> ^
       --version 2026.10.28.1 --eqhost <login server host>:5999 ^
       --patcher VoidPatcher\bin\Release\net48\VoidPatcher.exe ^
       --message "Welcome to the Voidforged test."
   ```

   - `--eqhost` writes `eqhost.txt` for players, so it does not matter what the reference client has.
   - `--patcher` publishes a self-update. Leave it out to keep the current patcher.
   - `--message` lines show at the top of the patcher window. You can pass it more than once.

3. Upload `objects/` first and `manifest.txt` last, so players never get a list naming files the host
   does not have yet. With rclone and a remote named `r2`:

   ```
   rclone copy  <publish folder>\objects     r2:voidforged-patch/objects --ignore-existing --transfers 16
   rclone copyto <publish folder>\manifest.txt r2:voidforged-patch/manifest.txt
   ```

To retire a file the client used to ship, add `delete <path>` to `patch_rules.txt` and rebuild.

## Cloudflare R2 setup (one time)

R2 charges nothing for downloads, which is what matters for a 10 GB client. Storage beyond the free
10 GB is about $0.015 per GB per month.

1. Create a Cloudflare account and open **R2**. It asks for a payment method even on the free tier.
2. Create a bucket, for example `voidforged-patch`.
3. Make it public. Under the bucket's **Settings**, either:
   - connect a custom domain such as `patch.yourdomain.com` (best, but the domain must be on Cloudflare), or
   - enable the `r2.dev` public URL. This is fine for a small test; Cloudflare rate limits it.
4. Under **R2 > Manage API tokens**, create a token with Object Read & Write on that bucket. Note the
   Access Key ID, Secret Access Key, and the S3 endpoint `https://<account id>.r2.cloudflarestorage.com`.
5. Install rclone (`winget install Rclone.Rclone`) and run `rclone config`:
   - new remote `r2`, storage type `s3`, provider `Cloudflare`;
   - paste the access key and secret;
   - endpoint is the URL from step 4.
6. Put the public URL from step 3 into `DefaultUrl` in `Program.cs`, rebuild, and publish with `--patcher`.

## Building the patcher

```
dotnet build VoidPatcher -c Release
```

The exe lands in `VoidPatcher\bin\Release\net48\VoidPatcher.exe`. It is a single file with no DLLs.

For a scripted run, or for testing, `VoidPatcher.exe --nogui` patches without a window and exits 0 on
success. Add `--verify` to ignore the hash cache and recheck every file.
