#!/bin/sh
# Writes the server config from the environment, then starts shared_memory, world and the zone launcher.
set -e
mkdir -p /data/shared /data/logs
cd /data

for x in quests lua_modules maps plugins; do ln -sfn "/runtime/$x" "$x"; done
for b in world zone eqlaunch shared_memory; do ln -sfn "/opt/vf/bin/$b" "$b"; done
cp /opt/vf/patches/patch_*.conf .
# opcodes.conf and any server-specific patch files from the runtime folder win
cp /runtime/patch_*.conf /runtime/opcodes.conf . 2>/dev/null || true

cat > eqemu_config.json <<EOF
{"server":{
  "zones":{"defaultstatus":"0","ports":{"low":"7001","high":"7030"}},
  "database":{"host":"db","port":"3306","db":"${DB_NAME}","username":"root","password":"${DB_ROOT_PASSWORD}"},
  "qsdatabase":{"host":"db","port":"3306","db":"${DB_NAME}","username":"root","password":"${DB_ROOT_PASSWORD}"},
  "world":{
    "shortname":"${SERVER_SHORTNAME}","longname":"${SERVER_LONGNAME}",
    "address":"${PUBLIC_ADDRESS}","localaddress":"${PUBLIC_ADDRESS}",
    "key":"${WORLD_KEY}",
    "tcp":{"ip":"0.0.0.0","port":"9005"},"http":{"enabled":"false"},
    "loginserver":{"host":"${LOGIN_HOST}","port":"${LOGIN_PORT}","legacy":"0","account":"","password":""}
  },
  "chatserver":{"host":"${PUBLIC_ADDRESS}","port":"7778"},
  "mailserver":{"host":"${PUBLIC_ADDRESS}","port":"7778"}
}}
EOF
chmod 600 eqemu_config.json

./shared_memory > logs/shared_memory.out 2>&1
./world > logs/world.out 2>&1 &
# world applies any database updates before zones may connect
sleep "${WORLD_START_SECONDS:-30}"
./eqlaunch zone > logs/launcher.out 2>&1 &
wait
