#ifndef ENTROPY_HYPERGRAPH_H
#define ENTROPY_HYPERGRAPH_H

#include "HyperGraph_Partitioning_Parser.h"

struct entropy_node{
    uint32_t id;
    uint8_t tier;             // tier = -1 is invalid
    double size;
    double Gain;
};
struct entropy_net{
    uint32_t id;
    double Current_Entropy;
    double Pre_Entropy;
    uint64_t prob_ptr;        // prob_ptr = 0 is invalid
    double max_size;
    double Entropy_weight;
};

struct Heap_t{
    double profit;
    uint32_t node_id;
    uint8_t next_tier;
    uint32_t heap_index;           // it directs, where the actual heap element resides currently
};

void Heap_Decrease_Key(Heap_t* Max_Heap, uint32_t node_index, uint32_t Heap_Size);
void Heap_Increase_Key(Heap_t* Max_Heap, uint32_t node_index, uint32_t Heap_Size);
Heap_t get_Best_Element(Heap_t* Max_Heap, uint32_t Heap_Size, const set<uint32_t>& Lock_set);
void Heap_element_swap(Heap_t* Max_Heap, uint32_t first_ele_index, uint32_t second_ele_index);

class Entropy_HyperGraph{
    private:
        HyperGraph_Partitioning_Parser* Parser;
        HyperGraph* HG = 0;
        vector<block_node>* Block_Nodes = 0;
        vector<string>* Node_id_to_Name = 0;
        vector<entropy_node>* Entropy_Nodes = 0;
        vector<entropy_net>* Entropy_Nets = 0;
        vector<double> *Prob_list = 0;
        bool initial_partition_done;
        bool entropy_initialized;
        uint8_t No_of_Tiers;
        double Total_area;
        uint8_t* Partition = 0;
        double THW;
        double* Gain_List = 0;
        Heap_t* Max_Heap = 0;
    public:
        explicit Entropy_HyperGraph(){};
        Entropy_HyperGraph(HyperGraph_Partitioning_Parser* Parser);
        ~Entropy_HyperGraph();
        void load_HyperGraph(uint8_t No_of_Tiers);
        void load_HyperGraph(uint8_t No_of_Tiers, string file);
        void load_Initial_Partition(string file);
        void initial_partition();
        void greedy_initial_partition();
        void Dump_initial_partition(string dumpfile, string write_string);
        void Dump_partition(string dumpfile, string write_string);
        void Modified_Fiduccia_Mattheyses(int repetation);
        void Area_Coverage(float allowed_percentage);
        void Simulated_Annelation();
        void Shuffled_Partition(int repetation);
        uint32_t blocks_size();
        bool Keep_ledger(uint32_t count);
        void load_Partition();
        [[deprecated]] void Print_net_entropies(string file);
        [[deprecated]] void print_net_connections();
        [[deprecated]] void print_block_adjacency();
    private:
        double Calculate_Max_Size_of_net(uint32_t net_id);
        uint32_t initial_block_index(const set<uint32_t> *unplaced_set);
        uint8_t initial_tier_index(const uint32_t block_id, const set<uint32_t> *unplaced_set);
        double get_weighted_conectivity(const uint32_t block_id, const set<uint32_t> *unplaced_set);
        void initialize_Prob_list();
        void calculate_Entropy();
        void calculate_entropy_of(uint32_t net_id);
        void calculate_Gain();
        void calculate_gain_of(uint32_t node_id);
        double gain_at_tier(uint32_t node_id, uint8_t tier_index);
        void place_block_at_tier(uint32_t block_id, uint8_t tier_index);
        void initialize_Entropy();
        double entropy(const vector<double>& Prob, const uint8_t base);
        double gain(const vector<uint32_t>& connected_nets, const vector<double>& entropy_list);
        double gain_increment_for(uint32_t net_id);
        double Calculate_THW();
        uint64_t Calculate_MIV();
        void max_area_coverage(double* Area, double max_area, uint8_t tier_index);
        void min_area_coverage(double* Area, double min_area, uint8_t tier_index);
        double weighted_conectivity_at_tier_level(uint32_t node_id);
        double Transition(double THw, double* Size_list, uint32_t node_id);
        void Initialize_Gain_List();
        void copy_Partition();
        const vector<double>& calculate_gain_list_of(uint32_t node_id, uint32_t net_id, const vector<double>& prob);
        const vector<double>& calculate_gain_list_of(uint32_t node_id, uint32_t net_id, uint8_t new_tier, const vector<double>& prob);
        void Initialize_Heap();
        void resettle_node_in_Heap(Heap_t* Max_Heap, uint32_t node_id, uint32_t Heap_Size);
};


#endif