#ifndef HYPERGRAPH_ENTROPY_PARTITIONER_H
#define HYPERGRAPH_ENTROPY_PARTITIONER_H

#include"Entropy_HyperGraph.h"

class HyperGraph_Entropy_Partioner{
    private:
        vector<Entropy_HyperGraph>* EHG_list = 0;
        HyperGraph_Partitioning_Parser* Parser = 0;
        vector<thread> thread_list;
        const char* FILE;
    public:
        explicit HyperGraph_Entropy_Partioner(){};
        ~HyperGraph_Entropy_Partioner(){
            delete Parser;
            delete EHG_list;
        };
        HyperGraph_Entropy_Partioner(const char* blocks_file, const char* nets_file, const char* benchmark)
        :FILE(blocks_file){
            Parser = new HyperGraph_Partitioning_Parser(blocks_file, nets_file, benchmark);
            Parser->build_HyperGraph();
        }
        void init_simulated();
        void init(uint8_t max_No_of_Tiers);
        void init(uint8_t max_No_of_Tiers, string IP_name);
        void initialize_partition(Entropy_HyperGraph& EHG, uint8_t nop);
        void initialize_partition(Entropy_HyperGraph& EHG, uint8_t nop, string IP_file);
        void run_Entropy_Partition_Package(Entropy_HyperGraph& EHG, uint8_t nop, float relax_percentage);
        void run_Entropy_Partition_Package(Entropy_HyperGraph& EHG, uint8_t nop);
    private:
        string load_initial_partition(Entropy_HyperGraph& EHG, uint8_t nop);
        
};

#endif