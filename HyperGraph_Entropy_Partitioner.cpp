#include"HyperGraph_Entropy_Partitioner.h"

void HyperGraph_Entropy_Partioner::init(uint8_t max_No_of_Tiers){
    for(uint8_t i = max_No_of_Tiers ; i >= 2; i -= 1){
        Entropy_HyperGraph EHG = Entropy_HyperGraph(this->Parser);
        EHG.load_HyperGraph(i);
        cout<<"HyperGraph loaded"<<endl;
        string IP_file = load_initial_partition(EHG, i);
        initialize_partition(EHG, i, IP_file);
    }
}

void HyperGraph_Entropy_Partioner::init_simulated(){
    Entropy_HyperGraph EHG = Entropy_HyperGraph(this->Parser);
    EHG.load_HyperGraph(2);
    cout<<"HyperGraph loaded"<<endl;
    auto start = chrono::high_resolution_clock::now();
    EHG.Simulated_Annelation();
    auto stop = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(stop - start);
    string write_string;
    string head = string(FILE).substr(13);
    head = string(head).substr(0,string(head).size()-7);
    //cout<<"head = "<<head<<endl;
    string dumpfile = "./MFM/"+head+"_Simulated_"+".SA";
    write_string = "SA time taken in miliseconds = "+to_string(duration.count())+"\n";
    EHG.Dump_initial_partition(dumpfile, write_string);
    //EHG.Dump_partition(dumpfile, write_string);
}


string HyperGraph_Entropy_Partioner::load_initial_partition(Entropy_HyperGraph& EHG, uint8_t nop){
    string head = string(FILE).substr(13);
    head = string(head).substr(0,string(head).size()-7);
    string IP_file = "./IP/"+head+"_greedy_"+to_string(nop)+".ip";
    auto start = chrono::high_resolution_clock::now();
    //EHG.initial_partition();
    EHG.greedy_initial_partition();
    auto stop = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(stop - start);
    string write_string;
    write_string = "Initial Partition time taken in miliseconds = "+to_string(duration.count())+"\n";
    EHG.Dump_initial_partition(IP_file, write_string);
    cout<<"Partitioning done from scratch . ip_file = "<<IP_file<<endl;
    return IP_file;
}

void HyperGraph_Entropy_Partioner::init(uint8_t max_No_of_Tiers, string IP_name){
    for(uint8_t i = 1 ; i < 2; i++){
        Entropy_HyperGraph EHG = Entropy_HyperGraph(this->Parser);
        string IP_file = IP_name+"_"+to_string(i+1)+".ip";
        cout<<"Initializing from file = "<<IP_file<<endl;
        EHG.load_HyperGraph(i+1, IP_file);
        cout<<"HyperGraph loaded"<<endl;
        initialize_partition(EHG, i+1, IP_file);
    }

}

void HyperGraph_Entropy_Partioner::initialize_partition(Entropy_HyperGraph& EHG, uint8_t nop, string IP_file){
    run_Entropy_Partition_Package(EHG, nop, 0.20);
    EHG.load_HyperGraph(nop, IP_file);
    run_Entropy_Partition_Package(EHG, nop, 0.15);
    EHG.load_HyperGraph(nop, IP_file);
    run_Entropy_Partition_Package(EHG, nop, 0.10);
    EHG.load_HyperGraph(nop, IP_file);
    run_Entropy_Partition_Package(EHG, nop, 0.05);
}

void HyperGraph_Entropy_Partioner::initialize_partition(Entropy_HyperGraph& EHG, uint8_t nop){
    run_Entropy_Partition_Package(EHG, nop);

}

void HyperGraph_Entropy_Partioner::run_Entropy_Partition_Package(Entropy_HyperGraph& EHG, uint8_t nop, float relax_percentage){
    uint32_t Blocks_count = EHG.blocks_size();
    cout<<"WS Relax = "<<to_string(relax_percentage)<<endl;
    auto start = chrono::high_resolution_clock::now();
    for(int rep = 0; rep < (int)log(Blocks_count); rep++){
        cout<<"Calling MFM"<<endl;
        EHG.Modified_Fiduccia_Mattheyses(nop*Blocks_count);
        cout<<"Calling Area coverage"<<endl;
        EHG.Area_Coverage(relax_percentage);
        if(rep == 0){
            EHG.Keep_ledger(0);
            auto end = chrono::high_resolution_clock::now();
            auto dur = chrono::duration_cast<chrono::milliseconds>(end - start);
            string write_string;
            string head = string(FILE).substr(13);
            head = string(head).substr(0,string(head).size()-7);
            string dumpfile = "./MFM/"+head+"_greedy_WS__tier_"+to_string(nop)+"_relax_"+to_string(relax_percentage)+".MFM";
            write_string = "MFM_WS time taken in miliseconds = "+to_string(dur.count())+"\n";
            EHG.Dump_partition(dumpfile, write_string);
        }
        if(EHG.Keep_ledger(rep)){
            EHG.Shuffled_Partition((int)log(Blocks_count));
        }
        else{
            EHG.load_Partition();
            cout<<"  Keep ledger exited the loop"<<endl;
            break;
        }
    }
    auto stop = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(stop - start);
    string write_string;
    string head = string(FILE).substr(13);
    head = string(head).substr(0,string(head).size()-7);
    //cout<<"head = "<<head<<endl;
    string dumpfile = "./MFM/"+head+"_greedy_tier_"+to_string(nop)+"_relax_"+to_string(relax_percentage)+".MFM";
    write_string = "MFM time taken in miliseconds = "+to_string(duration.count())+"\n";
    EHG.Dump_partition(dumpfile, write_string);
    cout<<"\nRelaxation = "<<to_string(relax_percentage)<<" done. Stored in "<<dumpfile<<endl;
}

void HyperGraph_Entropy_Partioner::run_Entropy_Partition_Package(Entropy_HyperGraph& EHG, uint8_t nop){
    uint16_t Blocks_count = EHG.blocks_size();
    auto start = chrono::high_resolution_clock::now();
    for(int rep = 0; rep < (int)log(Blocks_count); rep++){
        cout<<"Calling MFM"<<endl;
        EHG.Modified_Fiduccia_Mattheyses(Blocks_count);
        if(EHG.Keep_ledger(rep)){
            EHG.Shuffled_Partition((int)log(Blocks_count));
        }
        else{
            EHG.load_Partition();
            cout<<"  Keep ledger exited the loop"<<endl;
            break;
        }
    }
    auto stop = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(stop - start);
    string write_string;
    string head = string(FILE).substr(13);
    head = string(head).substr(0,string(head).size()-7);
    //cout<<"head = "<<head<<endl;
    string dumpfile = "./MFM/"+head+"_tier_"+to_string(nop)+"_norelax_"+".MFM";
    write_string = "MFM time taken in miliseconds = "+to_string(duration.count())+"\n";
    EHG.Dump_partition(dumpfile, write_string);
    cout<<"\nSecondary Partition done. Stored in "<<dumpfile<<endl;
}