# Compiles (does not run) Lua files with the server's own lua51.dll, reporting syntax errors.
import ctypes, sys

lua = ctypes.CDLL(sys.argv[1])
lua.luaL_newstate.restype = ctypes.c_void_p
lua.luaL_loadbuffer.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_size_t, ctypes.c_char_p]
lua.lua_tolstring.argtypes = [ctypes.c_void_p, ctypes.c_int, ctypes.c_void_p]
lua.lua_tolstring.restype = ctypes.c_char_p
lua.lua_settop.argtypes = [ctypes.c_void_p, ctypes.c_int]
L = lua.luaL_newstate()

bad = 0
for path in sys.argv[2:]:
    src = open(path, 'rb').read()
    if lua.luaL_loadbuffer(L, src, len(src), ('@' + path).encode()) != 0:
        bad += 1
        print('ERROR', lua.lua_tolstring(L, -1, None).decode(errors='replace'))
    else:
        print('ok   ', path)
    lua.lua_settop(L, 0)
sys.exit(1 if bad else 0)
