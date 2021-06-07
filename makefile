quantize.so : quant.o pch.o quantize.o
		g++ -shared quant.o pch.o quantize.o -o quantize.so

quantize.o : quantize.cpp
		g++ -fPIC -I /usr/include/python3.7 -I /home/zzaddp/.local/lib/python3.7/site-packages/numpy/core/include -c quantize.cpp 

pch.o : pch.cpp pch.h
		g++ -fPIC -I /usr/include/python3.7 -I /home/zzaddp/.local/lib/python3.7/site-packages/numpy/core/include -c pch.cpp

quant.o : quant.h quant.cpp
		g++ -fPIC -c quant.cpp

clean : 
		rm quant.o pch.o quantize.o quantize.so