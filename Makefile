mapm.dll: lmapm.c
	gcc -c lmapm.c $(ARG) -Ic:/ -DDLUA_BUILD_AS_DLL -O2 -o mapm.o
	gcc -shared -o mapm.dll mapm.o d:/lua/lua51.dll -Lc:/mapm -lmapm
	strip mapm.dll

clean:;
	del *.o *.dll