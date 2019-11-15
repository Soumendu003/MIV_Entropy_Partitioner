CXX = g++
CXXFLAGS = -std=c++11 -Wall -g -lpthread

CURR_DIR = /home/Guddu/MIV_Entropy_Partitioner
UTIL_DIR = /home/Guddu/Util
OBJECTS = $(addsuffix .o, $(basename $(wildcard *.cpp)))
INCLUDE_DIR = ./build

install: copy

%.o : %.cpp
	$(CXX) $(CXXFLAGS) -c $^

copy: $(UTIL_DIR)
	rm -rf $(INCLUDE_DIR)
	mkdir $(INCLUDE_DIR)
	cp $(UTIL_DIR)/* $(INCLUDE_DIR)
	cp -f ./*.* $(INCLUDE_DIR)
	$(MAKE) -C $(INCLUDE_DIR) -f Makefile.mk build

build: $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o exe
	cp exe.exe ./..
	$(MAKE) -C $(CURR_DIR) -f Makefile.mk clean

clean:
	rm -rf $(INCLUDE_DIR)

run_batch0:
	./exeH.exe -blocks ./benchmarks/ami33.blocks -nets ./benchmarks/ami33.nets -benchmark opencore_soft 
	./exeH.exe -blocks ./benchmarks/ami49.blocks -nets ./benchmarks/ami49.nets -benchmark opencore_hard 
	./exeH.exe -blocks ./benchmarks/n100.blocks -nets ./benchmarks/n100.nets -benchmark opencore_hard
	./exeH.exe -blocks ./benchmarks/n200.blocks -nets ./benchmarks/n200.nets -benchmark opencore_hard
	./exeH.exe -blocks ./benchmarks/n300.blocks -nets ./benchmarks/n300.nets -benchmark opencore_hard

run_batch_SA:
	./exe.exe -blocks ./benchmarks/aes_128.blocks -nets ./benchmarks/aes_128.n -benchmark ispd -SA
	#./exe.exe -blocks ./benchmarks/cf_fft_256_8.blocks -nets ./benchmarks/cf_fft_256_8.n -benchmark ispd -SA
	#./exe.exe -blocks ./benchmarks/cf_ldpc.blocks -nets ./benchmarks/cf_ldpc.n -benchmark ispd -SA

run_batch_invalid:
	#./exe.exe -blocks ./benchmarks/aes_128.blocks -nets ./benchmarks/aes_128.n -benchmark ispd -SA
	./exe.exe -blocks ./benchmarks/cf_fft_256_8.blocks -nets ./benchmarks/cf_fft_256_8.n -benchmark ispd

run_batch2:
	./exe.exe -blocks ./benchmarks/cf_ldpc.blocks -nets ./benchmarks/cf_ldpc.n -benchmark ispd 
	./exe.exe -blocks ./benchmarks/cf_fft_256_8.blocks -nets ./benchmarks/cf_fft_256_8.n -benchmark ispd

run_batch3:
	./exe.exe -blocks ./benchmarks/IBM10C_L1_V1.blocks -nets ./benchmarks/IBM10C_L1_V1.n -benchmark ispd
	./exe.exe -blocks ./benchmarks/aes_128.blocks -nets ./benchmarks/aes_128.n -benchmark ispd
	


run_gdb_ter2:
	gdb --args exe1.exe -blocks ./benchmarks/n100.blocks -nets ./benchmarks/n100.nets -benchmark opencore_hard
	gdb --args exe1.exe -blocks ./benchmarks/n200.blocks -nets ./benchmarks/n200.nets -benchmark opencore_hard
	gdb --args exe1.exe -blocks ./benchmarks/n300.blocks -nets ./benchmarks/n300.nets -benchmark opencore_hard

run_gdb_ter1:
	gdb --args exe.exe -blocks ./benchmarks/IBM01A_L0.blocks -nets ./benchmarks/IBM01A_L0.n -benchmark ispd -ipname ./IP/ispd