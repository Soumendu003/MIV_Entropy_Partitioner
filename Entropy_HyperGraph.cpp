#include"Entropy_HyperGraph.h"

Entropy_HyperGraph::Entropy_HyperGraph(HyperGraph_Partitioning_Parser* Parser){
    this->Parser = Parser;
    //Prob_list = new vector<double>
    //Prob_list->push_back(0);
    initial_partition_done = false;
    entropy_initialized = false;
    No_of_Tiers = 0;
}

Entropy_HyperGraph::~Entropy_HyperGraph(){
    //delete HG;
    //delete Block_Nodes;
    //delete Node_id_to_Name;
    delete Entropy_Nodes;
    delete Entropy_Nets;
    delete Prob_list;
    if(Partition != 0){
        free(Partition);
    }
    if(Gain_List != 0){
        free(Gain_List);
    }
    if(Max_Heap != 0){
        free(Max_Heap);
    }
}

void Entropy_HyperGraph::load_HyperGraph(uint8_t No_of_Tiers){
    //Parser.build_HyperGraph();
    //cout<<"Load HyperGraph() called"<<endl;
    this->HG = Parser->copy_HyperGraph();
    this->Block_Nodes = Parser->copy_Block_Nodes();
    this->Node_id_to_Name = Parser->copy_Node_id_to_Name();
    this->Entropy_Nodes = new vector<entropy_node>;
    this->Entropy_Nets = new vector<entropy_net>;
    this->No_of_Tiers = No_of_Tiers;
    this->Total_area = 0;

    for(uint32_t i = 0; i < HG->get_Nodes_Size() ; i++){
        entropy_node tem;
        tem.id = i;
        tem.tier = -1;
        tem.Gain = 0;
        tem.size = Block_Nodes->at(i).height + Block_Nodes->at(i).width;
        //cout<<"Block id = "<<to_string(i)<<"\t height = "<<to_string(Block_Nodes->at(i).height)<<"\t width= "+to_string(Block_Nodes->at(i).width)<<endl;
        this->Total_area += (Block_Nodes->at(i).height)*(Block_Nodes->at(i).width);
        Entropy_Nodes->push_back(tem);
    }

    for(uint32_t i = 0; i < HG->get_Nets_Size(); i++){
        entropy_net tem;
        tem.id = i;
        tem.Current_Entropy = 0;
        tem.Pre_Entropy = 0;
        tem.prob_ptr = 0;
        tem.Entropy_weight = 0;
        tem.max_size = Calculate_Max_Size_of_net(i);
        Entropy_Nets->push_back(tem);
    }
}

void Entropy_HyperGraph::load_HyperGraph(uint8_t No_of_Tiers, string file){
    cout<<"Loading Hhyper_Graph. And loading Initial Partition"<<endl;
    load_HyperGraph(No_of_Tiers);
    load_Initial_Partition(file);
    entropy_initialized = false;
}

void Entropy_HyperGraph::load_Initial_Partition(string file){
    const char* delimeters = "{},\t ()\n\r";
    char end_char = '\n';
    cout<<"File name = "<<file<<endl;
    Tokenizer* Tok = new Tokenizer(file.c_str(), delimeters, end_char);
    char* tok = Tok->get_token();
    while(tok != 0){
        if(Strcmpi(tok, "Block_ID") == 0){
            Tok->get_token();
            uint32_t block_id = atoi(Tok->get_token());
            Tok->get_token();           // 'tier_index'
            Tok->get_token();           // '='
            uint8_t tier_index = atoi(Tok->get_token());
            Entropy_Nodes->at(block_id).tier = tier_index;
            Entropy_Nodes->at(block_id).Gain = 0;
        }
        else if(Strcmpi(tok, "Block") == 0){
            Tok->get_token();
            string block_name = string(Tok->get_token());
            uint32_t block_id = stoi(block_name.substr(2,block_name.size()-2));
            Tok->get_token();           // 'tier_index'
            Tok->get_token();           // '='
            uint8_t tier_index = atoi(Tok->get_token());
            Entropy_Nodes->at(block_id).tier = tier_index;
            Entropy_Nodes->at(block_id).Gain = 0;
        }
        tok = Tok->get_token();
    }
    for(uint32_t i = 0; i < HG->get_Nets_Size(); i++){
        Entropy_Nets->at(i).Current_Entropy = 0;
        Entropy_Nets->at(i).Pre_Entropy = 0;
        Entropy_Nets->at(i).prob_ptr = 0;
    }
    delete Tok;
}

void Entropy_HyperGraph::print_net_connections(){
    int S = HG->get_Nets_Size();
    for(int i = 0; i < S; i++){
        const vector<uint32_t> V = HG->get_connected_nodes_of(i);
        cout<<"Net id = "<<i<<endl;
        for(int j = 0; j < V.size(); j++){
            cout<<"Block = "<<Node_id_to_Name->at(V[j])<<endl;
        }
    }
}

void Entropy_HyperGraph::print_block_adjacency(){
    int S = Node_id_to_Name->size();
    for(int i = 0; i < S; i++){
        const vector<adjacent_node> V = HG->get_adjacent_nodes_of(i);
        cout<<"Block name = "<<Node_id_to_Name->at(i)<<endl;
        for(int j = 0; j < V.size(); j++)
        {
            cout<<"Adj_Block_Name = "<<Node_id_to_Name->at(V[j].id)<<"\t connection_count = "<<V[j].common_net_count<<endl;
        }
    }

}

double Entropy_HyperGraph::Calculate_Max_Size_of_net(uint32_t net_id){
    const vector<uint32_t> connected_nodes = HG->get_connected_nodes_of(net_id);
    double sum = 0 ;
    for(uint32_t i = 0; i < connected_nodes.size(); i++){
        sum += Entropy_Nodes->at(connected_nodes[i]).size;
    }
    return sum;
}

double Entropy_HyperGraph::weighted_conectivity_at_tier_level(uint32_t node_id){
    const vector<adjacent_node> Adjacent_Nodes = HG->get_adjacent_nodes_of(node_id);
    double weighted_connectivity = 0;
    double total_connectivity = 0;
    double Block_Size = Entropy_Nodes->at(node_id).size;
    double factor;
    uint32_t adj_node;
    for(uint32_t i = 0; i < Adjacent_Nodes.size(); i++){
        adj_node = Adjacent_Nodes[i].id;
        if(Entropy_Nodes->at(adj_node).tier == Entropy_Nodes->at(node_id).tier){
            total_connectivity += Adjacent_Nodes[i].common_net_count;
        }
    }
    if(total_connectivity == 0){
        return 0;
    }
    for(uint32_t i = 0; i < Adjacent_Nodes.size(); i++){
        adj_node = Adjacent_Nodes[i].id;
        if(Entropy_Nodes->at(adj_node).tier == Entropy_Nodes->at(node_id).tier){
            factor = (Entropy_Nodes->at(adj_node).size + (Adjacent_Nodes[i].common_net_count/total_connectivity));
            weighted_connectivity += Adjacent_Nodes[i].common_net_count*factor*factor;
        }
    }
    return weighted_connectivity;
}

double Entropy_HyperGraph::get_weighted_conectivity(const uint32_t block_id , const set<uint32_t> *unplaced_set){
    const vector<adjacent_node> Adjacent_Nodes = HG->get_adjacent_nodes_of(block_id);
    double weighted_connectivity = 0;
    double total_connectivity = 0;
    double Block_Size = Entropy_Nodes->at(block_id).size;
    double factor;
    set<uint32_t>::iterator it;
    if(unplaced_set->size() == HG->get_Nodes_Size()){
        for (uint32_t j = 0; j < Adjacent_Nodes.size(); j++)
        {
            total_connectivity += Adjacent_Nodes[j].common_net_count;
        }
        if(total_connectivity == 0){
            return 0;
        }
        for(uint32_t j = 0; j < Adjacent_Nodes.size(); j++){
            factor = (Entropy_Nodes->at(Adjacent_Nodes[j].id).size + (Adjacent_Nodes[j].common_net_count/total_connectivity)*Block_Size);
            weighted_connectivity += Adjacent_Nodes[j].common_net_count*factor*factor;
        }
    }
    else{
        for (uint32_t j = 0; j < Adjacent_Nodes.size(); j++){
            it = unplaced_set->find(Adjacent_Nodes[j].id);
            if(it == unplaced_set->end()){
                total_connectivity += Adjacent_Nodes[j].common_net_count;
            }
        }
        if(total_connectivity == 0){
            return 0;
        }
        for(uint32_t j = 0; j < Adjacent_Nodes.size(); j++){
            it = unplaced_set->find(Adjacent_Nodes[j].id);
            if(it == unplaced_set->end()){
                factor = (Entropy_Nodes->at(Adjacent_Nodes[j].id).size + (Adjacent_Nodes[j].common_net_count/total_connectivity)*Block_Size);
                weighted_connectivity += Adjacent_Nodes[j].common_net_count*factor*factor;
            }
        }
    }
    return weighted_connectivity;
}

uint32_t Entropy_HyperGraph::initial_block_index(const set<uint32_t> *unplaced_set){
    double Min_weighted_connectivity = -1;
    double weighted_connectivity;
    uint32_t block_id ;
    set<uint32_t>::iterator it;
    if(unplaced_set->size() == 1){
        it = unplaced_set->begin();
        block_id = *it;
    }
    else{
        for(it = unplaced_set->begin(); it != unplaced_set->end(); it++){
            weighted_connectivity = get_weighted_conectivity(*it, unplaced_set);
            if(weighted_connectivity == 0){
                return *it;
            }
            if(Min_weighted_connectivity < 0){
                Min_weighted_connectivity = weighted_connectivity;
                block_id = *it;
            }
            else if(weighted_connectivity < Min_weighted_connectivity){
                Min_weighted_connectivity = weighted_connectivity;
                block_id = *it;
            }
        }
        
    }
    return block_id;
}

uint8_t Entropy_HyperGraph::initial_tier_index(const uint32_t block_id, const set<uint32_t> *unplaced_set){
    if(unplaced_set->size() == HG->get_Nodes_Size()){
        return 0;
    }
    const vector<adjacent_node> Adjacent_Nodes = HG->get_adjacent_nodes_of(block_id);
    double* tier_coneectivity = (double*)calloc(No_of_Tiers, sizeof(double));
    double total_connectivity = 0;
    double factor;
    double Block_Size = Entropy_Nodes->at(block_id).size;
    set<uint32_t>::iterator it;
    for (uint32_t j = 0; j < Adjacent_Nodes.size(); j++){
        it = unplaced_set->find(Adjacent_Nodes[j].id);
        if(it == unplaced_set->end()){
            total_connectivity += Adjacent_Nodes[j].common_net_count;
        }
    }
    if(total_connectivity == 0){
        //cout<<"Block_id = "<<to_string(block_id)<<"\t total_connectivity = 0"<<endl;
        return 0;
    }

    for(uint32_t j = 0; j < Adjacent_Nodes.size(); j++){
        it = unplaced_set->find(Adjacent_Nodes[j].id);
        if(it == unplaced_set->end()){
            factor = (Entropy_Nodes->at(Adjacent_Nodes[j].id).size + (Adjacent_Nodes[j].common_net_count/total_connectivity)*Block_Size);
            tier_coneectivity[Entropy_Nodes->at(Adjacent_Nodes[j].id).tier] += Adjacent_Nodes[j].common_net_count*factor*factor;
        }

    }
    uint8_t tier_index = 0;
    //cout<<"tier index = 0 \t connectivity = "<<tier_coneectivity[tier_index]<<endl;
    for(uint8_t i = 1; i < No_of_Tiers; i++){
        //cout<<"tier index = "<<to_string(i)<<" \t connectivity = "<<tier_coneectivity[tier_index]<<endl;
        if(tier_coneectivity[i] < tier_coneectivity[tier_index]){
            tier_index = i;
        }
    }
    free(tier_coneectivity);
    return tier_index;
}

void Entropy_HyperGraph::initial_partition(){
    if(initial_partition_done){
        throw HyperGraph_Exception(("Initial Partioning Already Done with no of Tiers = "+to_string(this->No_of_Tiers)+" . Can't do it twice").c_str());
    }
    set<uint32_t> *unplaced_set = new set<uint32_t>;

    for(uint32_t i = 0; i < HG->get_Nodes_Size(); i++){
        unplaced_set->insert(i);
    }    

    cout<<"Starting Initial Partition"<<endl;
    cout<<"Size of HG = "<<to_string(HG->get_Nodes_Size())<<endl;

    for(uint32_t i = 0; i < HG->get_Nodes_Size(); i++){
        uint32_t block_id = initial_block_index(unplaced_set);
        uint8_t tier_index = initial_tier_index(block_id, unplaced_set);
        unplaced_set->erase(block_id);
        Entropy_Nodes->at(block_id).tier = tier_index;
        //cout<<"Placed block = "<<to_string(block_id)<<"\t tier = "<<to_string(tier_index)<<endl;
    }
    delete unplaced_set;
    initial_partition_done = true;
    entropy_initialized = false;
}

void Entropy_HyperGraph::greedy_initial_partition(){
    if(initial_partition_done){
        throw HyperGraph_Exception(("Initial Partioning Already Done with no of Tiers = "+to_string(this->No_of_Tiers)+" . Can't do it twice").c_str());
    }
    set<uint32_t> *unplaced_set = new set<uint32_t>;

    for(uint32_t i = 0; i < HG->get_Nodes_Size(); i++){
        unplaced_set->insert(i);
    }    

    cout<<"Starting Greedy Initial Partition"<<endl;
    cout<<"Size of HG = "<<to_string(HG->get_Nodes_Size())<<endl;

    for(uint32_t i = 0; i < HG->get_Nodes_Size(); i++){
        uint32_t block_id = i;
        uint8_t tier_index = initial_tier_index(block_id, unplaced_set);
        unplaced_set->erase(block_id);
        Entropy_Nodes->at(block_id).tier = tier_index;
        //cout<<"Placed block = "<<to_string(block_id)<<"\t tier = "<<to_string(tier_index)<<endl;
    }
    delete unplaced_set;
    initial_partition_done = true;
    entropy_initialized = false;
}

void Entropy_HyperGraph::Dump_initial_partition(string dumpfile, string write_string){
    ofstream ofs;
    ofs.open(dumpfile, ofstream::out);
    ofs.write(write_string.c_str(), strlen(write_string.c_str()));
    write_string = "\nTHW = "+to_string(Calculate_THW())+"\n\n";
    ofs.write(write_string.c_str(), strlen(write_string.c_str()));
    write_string = "\nTotal area of all blocks  = "+to_string(Total_area)+"\n\n";
    ofs.write(write_string.c_str(), strlen(write_string.c_str()));
    write_string = "\nTotal no of MIV  = "+to_string(Calculate_MIV())+"\n\n";
    ofs.write(write_string.c_str(), strlen(write_string.c_str()));

    double avg_area = Total_area / No_of_Tiers;
    double* Area = (double*)calloc(No_of_Tiers,sizeof(double));
    for (uint32_t i = 0; i < HG->get_Nodes_Size(); i++){
        Area[Entropy_Nodes->at(i).tier] += Block_Nodes->at(i).height*Block_Nodes->at(i).width;
    }
    for(uint8_t k = 0; k < No_of_Tiers; k++){
        double div = abs((Area[k] - avg_area)/avg_area);
        write_string = "Divergence of area in tier = "+to_string(k)+" is "+to_string(div)+"\n";
        ofs.write(write_string.c_str(), strlen(write_string.c_str()));
    }

    for(uint32_t i = 0; i < HG->get_Nodes_Size(); i++){
        write_string = "Block_ID = "+to_string(i)+"\t tier_index = "+to_string(Entropy_Nodes->at(i).tier)+"\n";
        ofs.write(write_string.c_str(), strlen(write_string.c_str()));
    }
    ofs.close();
    return;
}


void Entropy_HyperGraph::Dump_partition(string dumpfile, string write_string){
    ofstream ofs;
    ofs.open(dumpfile, ofstream::out);
    ofs.write(write_string.c_str(), strlen(write_string.c_str()));

    copy_Partition();
    cout<<"Partition copied"<<endl;
    write_string = "\nTHW = "+to_string(Calculate_THW())+"\n\n";
    ofs.write(write_string.c_str(), strlen(write_string.c_str()));
    write_string = "\nTotal area of all blocks  = "+to_string(Total_area)+"\n\n";
    ofs.write(write_string.c_str(), strlen(write_string.c_str()));
    write_string = "\nTotal no of MIV  = "+to_string(Calculate_MIV())+"\n\n";
    ofs.write(write_string.c_str(), strlen(write_string.c_str()));

    double avg_area = Total_area / No_of_Tiers;
    double* Area = (double*)calloc(No_of_Tiers,sizeof(double));
    for (uint32_t i = 0; i < HG->get_Nodes_Size(); i++){
        Area[Entropy_Nodes->at(i).tier] += Block_Nodes->at(i).height*Block_Nodes->at(i).width;
    }
    for(uint8_t k = 0; k < No_of_Tiers; k++){
        double div = abs((Area[k] - avg_area)/avg_area);
        write_string = "Divergence of area in tier = "+to_string(k)+" is "+to_string(div)+"\n";
        ofs.write(write_string.c_str(), strlen(write_string.c_str()));
    }

    for(uint32_t i = 0; i < HG->get_Nodes_Size(); i++){
        write_string = "Block_ID = "+to_string(i)+"\t tier_index = "+to_string(Entropy_Nodes->at(i).tier)+"\n";
        ofs.write(write_string.c_str(), strlen(write_string.c_str()));
    }
    ofs.close();
    return;
}

void Entropy_HyperGraph::copy_Partition(){
    for(uint32_t i = 0; i < HG->get_Nodes_Size(); i++){
        Entropy_Nodes->at(i).tier = Partition[i];
    }    
}

void Entropy_HyperGraph::initialize_Prob_list(){
    delete Prob_list;
    Prob_list = new vector<double>;
    Prob_list->push_back(0);
    double net_size;
    uint64_t prob_ptr;
    uint8_t tier_index;
    double block_size;
    for(uint32_t i = 0; i < HG->get_Nets_Size(); i++){
        const vector<uint32_t> connected_nodes = HG->get_connected_nodes_of(i);
        prob_ptr = Prob_list->size();
        Entropy_Nets->at(i).prob_ptr = prob_ptr;
        net_size = Entropy_Nets->at(i).max_size;
        for(uint8_t k = 0; k < No_of_Tiers ; k++){
            Prob_list->push_back(0);
        }
        for(uint32_t j = 0; j < connected_nodes.size(); j++){
            tier_index = Entropy_Nodes->at(connected_nodes[j]).tier;
            block_size = Entropy_Nodes->at(connected_nodes[j]).size;
            Prob_list->at(prob_ptr + tier_index) += (block_size/net_size);
        }
    }
    cout<<"Prob_list initialized"<<endl;
}

double Entropy_HyperGraph::entropy(const vector<double>& Prob, const uint8_t base){
    double entropy = 0;
    if(base > 1){
        for(uint8_t k = 0; k < No_of_Tiers; k++){
            if(Prob[k] > 0 && Prob[k] < 1){
                entropy += (-1)*(Prob[k])*(log(Prob[k])/log(base));
            }
        }
    }
    else if(base == 1){
        entropy = 1;
    }
    else{
        entropy  = 1;
        //throw HyperGraph_Exception("Base can't be < 1 while calculating entropy");
    }
    return entropy;
}

void Entropy_HyperGraph::calculate_entropy_of(uint32_t net_id){
    Entropy_Nets->at(net_id).Pre_Entropy = Entropy_Nets->at(net_id).Current_Entropy;
    uint8_t base = Min(No_of_Tiers, HG->get_degree_of_net(net_id));
    vector<double> prob;
    for(uint8_t k = 0; k < No_of_Tiers; k++){
        prob.push_back(Prob_list->at(Entropy_Nets->at(net_id).prob_ptr + k));
    }
    Entropy_Nets->at(net_id).Current_Entropy = entropy(prob, base);
    //cout<<"Previous Entropy = "<<Entropy_Nets->at(net_id).Pre_Entropy<<endl;
    //cout<<"Current Entropy = "<<Entropy_Nets->at(net_id).Current_Entropy<<endl;
    if(Entropy_Nets->at(net_id).Current_Entropy < 0 || Entropy_Nets->at(net_id).Current_Entropy > 1){
        cout<<"Entropy value = "<<to_string(Entropy_Nets->at(net_id).Current_Entropy)<<endl;
        throw HyperGraph_Exception(("Entroy is out of range for net_id = "+to_string(net_id)).c_str());
    }
}

void Entropy_HyperGraph::calculate_Entropy(){
    double THW_2d = 0;
    for(uint32_t i = 0; i < HG->get_Nets_Size(); i++){
        //calculate_entropy_of(i);
        THW_2d += (Entropy_Nets->at(i).max_size)*(Entropy_Nets->at(i).max_size);
    }
    for(uint32_t i = 0; i < HG->get_Nets_Size(); i++){
        Entropy_Nets->at(i).Entropy_weight = ((Entropy_Nets->at(i).max_size)*(Entropy_Nets->at(i).max_size))/THW_2d;
        //cout<<"Entropy_weight["<<to_string(i)<<"] = "<<to_string(Entropy_Nets->at(i).Entropy_weight)<<endl;
    }
}

double Entropy_HyperGraph::gain(const vector<uint32_t>& connected_nets, const vector<double>& entropy_list){
    double gain = 0;
    for(uint32_t i = 0; i < connected_nets.size(); i++){
        //gain += entropy_list[i];
        //cout<<"Entropy weight = "<<to_string(Entropy_Nets->at(connected_nets[i]).Entropy_weight)<<endl;
        gain += Entropy_Nets->at(connected_nets[i]).Entropy_weight * entropy_list[i];
    }
    return gain;
}

double Entropy_HyperGraph::gain_increment_for(uint32_t net_id){
    //return (Entropy_Nets->at(net_id).Current_Entropy - Entropy_Nets->at(net_id).Pre_Entropy);
    return Entropy_Nets->at(net_id).Entropy_weight * (Entropy_Nets->at(net_id).Current_Entropy - Entropy_Nets->at(net_id).Pre_Entropy);
}

void Entropy_HyperGraph::calculate_gain_of(uint32_t node_id){
    const vector<uint32_t> connected_nets = HG->get_connected_nets_of(node_id);
    vector<double> entropy_list;
    for(uint32_t j = 0; j < connected_nets.size(); j++){
        entropy_list.push_back(Entropy_Nets->at(connected_nets[j]).Current_Entropy);
    }
    Entropy_Nodes->at(node_id).Gain = gain(connected_nets, entropy_list);
    if(Entropy_Nodes->at(node_id).Gain < 0){
        throw HyperGraph_Exception(("Gain is less than 0 for block_id = "+to_string(node_id)).c_str());
    }
    
}

const vector<double>& Entropy_HyperGraph::calculate_gain_list_of(uint32_t node_id, uint32_t net_id, uint8_t new_tier, const vector<double>& prob){
    if(prob.size() != No_of_Tiers){
        throw HyperGraph_Exception("prob list size not matched with no of Tiers.");
    }
    static vector<double> gain_list;
    gain_list.clear();
    uint8_t base = Min(HG->get_degree_of_net(net_id), No_of_Tiers);
    for(uint8_t k = 0; k < No_of_Tiers; k++){
        vector<double> tem_prob(prob);
        if(k == new_tier){
            //cout<<"Entropy weight = "<<to_string(Entropy_Nets->at(net_id).Entropy_weight)<<endl;
            gain_list.push_back(Entropy_Nets->at(net_id).Entropy_weight*entropy(tem_prob, base));
        }
        else{
            tem_prob[k] += (Entropy_Nodes->at(node_id).size/Entropy_Nets->at(net_id).max_size);
            tem_prob[new_tier] -= (Entropy_Nodes->at(node_id).size/Entropy_Nets->at(net_id).max_size);
            gain_list.push_back(Entropy_Nets->at(net_id).Entropy_weight*entropy(tem_prob, base));
        }
    }
    return gain_list;
}

const vector<double>& Entropy_HyperGraph::calculate_gain_list_of(uint32_t node_id, uint32_t net_id, const vector<double>& prob){
    if(prob.size() != No_of_Tiers){
        throw HyperGraph_Exception("prob list size not matched with no of Tiers.");
    }
    static vector<double> gain_list;
    gain_list.clear();
    vector<double> tem_prob;
    //cout<<"Size of gain_list = "<<to_string(gain_list.size())<<endl;
    uint8_t base = Min(HG->get_degree_of_net(net_id), No_of_Tiers);
    for(uint8_t k = 0; k < No_of_Tiers; k++){
        tem_prob.clear();
        for(uint8_t z = 0; z < prob.size(); z++){
            tem_prob.push_back(prob[z]);
        }
        if(k == Entropy_Nodes->at(node_id).tier){
            gain_list.push_back(Entropy_Nets->at(net_id).Entropy_weight*entropy(tem_prob, base));
        }
        else{
            tem_prob[k] += (Entropy_Nodes->at(node_id).size/Entropy_Nets->at(net_id).max_size);
            tem_prob[Entropy_Nodes->at(node_id).tier] -= (Entropy_Nodes->at(node_id).size/Entropy_Nets->at(net_id).max_size);
            gain_list.push_back(Entropy_Nets->at(net_id).Entropy_weight*entropy(tem_prob, base));
        }
    }
    return gain_list;
}

void Entropy_HyperGraph::Initialize_Gain_List(){
    if(Gain_List != 0){
        free(Gain_List);
    }
    Gain_List = (double*)calloc(HG->get_Nodes_Size()*No_of_Tiers, sizeof(double));

    uint16_t* Net_count = (uint16_t*)calloc(HG->get_Nodes_Size(), sizeof(uint16_t));

    vector<double> prob;
    uint64_t prob_ptr;
    for(uint32_t i = 0; i < HG->get_Nets_Size(); i++){
        prob_ptr = Entropy_Nets->at(i).prob_ptr;
        //cout<<"Prob ptr = "<<to_string(prob_ptr)<<endl;
        prob.clear();
        //cout<<"prob list cleared"<<endl;
        for(uint8_t k = 0; k < No_of_Tiers; k++){
            prob.push_back(Prob_list->at(prob_ptr + k));
        }
        uint8_t base = Min(HG->get_degree_of_net(i), No_of_Tiers);
        const vector<uint32_t> connected_nodes = HG->get_connected_nodes_of(i);
        for(uint32_t j = 0; j < connected_nodes.size(); j++){
            const vector<double> gain_list = calculate_gain_list_of(connected_nodes[j], i, prob);
            Net_count[connected_nodes[j]] += 1;
            for(uint8_t k = 0; k < No_of_Tiers; k++){
                //cout<<"gain_list["<<to_string(k)<<"] = "<<to_string(gain_list[k])<<endl;
                Gain_List[connected_nodes[j]*No_of_Tiers + k] += gain_list[k];
            }
        }
    }
}

void Entropy_HyperGraph::calculate_Gain(){
    for(uint32_t i = 0; i < HG->get_Nodes_Size(); i++){
        calculate_gain_of(i);
    }
}

void Entropy_HyperGraph::initialize_Entropy(){
    cout<<"   Initializing Entropy()  "<<endl;
    initialize_Prob_list();
    calculate_Entropy();
    //calculate_Gain();
    Initialize_Gain_List();
    cout<<"Gain list Initialized"<<endl;
    //Initialize_Heap();
    //cout<<"Heap Initialized"<<endl;
    entropy_initialized = true;
}

double Entropy_HyperGraph::gain_at_tier(uint32_t node_id, uint8_t tier_index){
    if(Entropy_Nodes->at(node_id).tier == tier_index){
        /*if(Entropy_Nodes->at(node_id).Gain < 0 || Entropy_Nodes->at(node_id).Gain > 1){
            cout<<("Gain value out of range for block = "+to_string(node_id)+"\t gain = "+to_string(Entropy_Nodes->at(node_id).Gain))<<endl;
            //throw HyperGraph_Exception();
        }*/
        return Entropy_Nodes->at(node_id).Gain;
    }
    uint8_t base;
    vector<double> net_prob;
    vector<double> entropy_list;
    const vector<uint32_t> connected_nets = HG->get_connected_nets_of(node_id);
    uint64_t Prob_ptr;
    double net_size;
    double block_size = Entropy_Nodes->at(node_id).size;
    for(uint32_t i = 0; i < connected_nets.size(); i++){
        Prob_ptr = Entropy_Nets->at(connected_nets[i]).prob_ptr;
        net_size = Entropy_Nets->at(connected_nets[i]).max_size;
        base = Min(No_of_Tiers, HG->get_degree_of_net(connected_nets[i]));
        for(uint8_t k = 0; k < No_of_Tiers; k++){
            if(k == tier_index){
                net_prob.push_back(Prob_list->at(Prob_ptr + k) + (block_size/net_size));
            }
            else if(k == Entropy_Nodes->at(node_id).tier){
                net_prob.push_back(Prob_list->at(Prob_ptr + k) - (block_size/net_size));
            }
            else{
                net_prob.push_back(Prob_list->at(Prob_ptr + k));
            }
        }
        entropy_list.push_back(entropy(net_prob, base));
        net_prob.clear();
    }
    double gain = this->gain(connected_nets, entropy_list);
    if(gain < 0){
        throw HyperGraph_Exception(("Returing gain < 0. From gain_at_tier("+to_string(node_id)+", "+to_string(tier_index)+")").c_str());
    }
    return gain;
}

void Entropy_HyperGraph::place_block_at_tier(uint32_t node_id, uint8_t tier_index){
    if(Entropy_Nodes->at(node_id).tier == tier_index){
        throw HyperGraph_Exception(("Can't place the block at its previous tier. block_id = "+to_string(node_id)+"\t pre_tier = "+to_string(tier_index)).c_str());
    }
    //cout<<"Before placing gain = "<<Entropy_Nodes->at(node_id).Gain<<endl;
    //set<uint32_t> modified_Block_set;
    uint8_t base;
    vector<double> pre_net_prob;
    vector<double> net_prob;
    const vector<uint32_t> connected_nets = HG->get_connected_nets_of(node_id);
    uint64_t Prob_ptr;
    double net_size;
    double block_size = Entropy_Nodes->at(node_id).size;
    for(uint32_t i = 0; i < connected_nets.size(); i++){
        Prob_ptr = Entropy_Nets->at(connected_nets[i]).prob_ptr;
        net_size = Entropy_Nets->at(connected_nets[i]).max_size;
        base = Min(No_of_Tiers, HG->get_degree_of_net(connected_nets[i]));
        for(uint8_t k = 0; k < No_of_Tiers; k++){
            pre_net_prob.push_back(Prob_list->at(Prob_ptr + k));
            if(k == tier_index){
                net_prob.push_back(Prob_list->at(Prob_ptr + k) + (block_size/net_size));
            }
            else if(k == Entropy_Nodes->at(node_id).tier){
                net_prob.push_back(Prob_list->at(Prob_ptr + k) - (block_size/net_size));
            }
            else{
                net_prob.push_back(Prob_list->at(Prob_ptr + k));
            }
            Prob_list->at(Prob_ptr + k) = net_prob[k];   
        }
        //Entropy_Nets->at(connected_nets[i]).Pre_Entropy = Entropy_Nets->at(connected_nets[i]).Current_Entropy;
        //Entropy_Nets->at(connected_nets[i]).Current_Entropy = this->entropy(net_prob, base);
        const vector<uint32_t> connected_nodes = HG->get_connected_nodes_of(connected_nets[i]);
        for(uint32_t j = 0; j < connected_nodes.size(); j++){
            //modified_Block_set.insert(connected_nodes[j]);
            //Entropy_Nodes->at(connected_nodes[j]).Gain += gain_increment_for(connected_nets[i]);
            if(connected_nodes[j] == node_id){
                const vector<double> old_gain_list = calculate_gain_list_of(connected_nodes[j], connected_nets[i], pre_net_prob);
                const vector<double> new_gain_list = calculate_gain_list_of(connected_nodes[j], connected_nets[i], tier_index, net_prob);
                for(uint8_t k = 0; k < No_of_Tiers; k++){
                    this->Gain_List[connected_nodes[j]*No_of_Tiers + k] -= old_gain_list[k];
                    this->Gain_List[connected_nodes[j]*No_of_Tiers + k] += new_gain_list[k];
                }
            }
            else{
                const vector<double> old_gain_list = calculate_gain_list_of(connected_nodes[j], connected_nets[i], pre_net_prob);
                const vector<double> new_gain_list = calculate_gain_list_of(connected_nodes[j], connected_nets[i], net_prob);
                for(uint8_t k = 0; k < No_of_Tiers; k++){
                    this->Gain_List[connected_nodes[j]*No_of_Tiers + k] -= old_gain_list[k];
                    this->Gain_List[connected_nodes[j]*No_of_Tiers + k] += new_gain_list[k];
                }
            }   
        }
        net_prob.clear();
        pre_net_prob.clear();
    }
    Entropy_Nodes->at(node_id).tier = tier_index;

    /*for(set<uint32_t>::iterator it = modified_Block_set.begin(); it != modified_Block_set.end(); it++){
        resettle_node_in_Heap(this->Max_Heap, *it, HG->get_Nodes_Size());
    }*/
}

void Entropy_HyperGraph::Modified_Fiduccia_Mattheyses(int rep){
    if(!entropy_initialized){
        initialize_Entropy();
    }
    uint64_t i;
    cout<<"Repetation size = "<<to_string(rep)<<endl;
    int Lock_Queue_size = (int)(log(HG->get_Nodes_Size())/log(2));
    queue<uint32_t> Lock_Queue;
    set<uint32_t> Lock_set;
    for(i = 0; i < rep ; i++){
        bool flag = true;
        double Max_Profit = 0, profit;
        uint32_t block_id;
        uint8_t tier_index;
        for(uint32_t j = 0; j < HG->get_Nodes_Size(); j++){
            if(Lock_set.find(j) == Lock_set.end()){
                for(uint8_t k = 0 ; k < No_of_Tiers; k++){
                    profit = Gain_List[j*No_of_Tiers + k] - Gain_List[j*No_of_Tiers + Entropy_Nodes->at(j).tier];
                    //cout<<"profit = "<<to_string(profit)<<endl;
                    //profit = gain_at_tier(j, k) - Entropy_Nodes->at(j).Gain;
                    if(flag){
                        Max_Profit = profit;
                        block_id = j;
                        tier_index = k;
                        flag = false;
                    }
                    else if(profit > Max_Profit){
                        Max_Profit = profit;
                        block_id = j;
                        tier_index = k;
                    }
                }
            }
        }

        /*Heap_t ele = get_Best_Element(this->Max_Heap, HG->get_Nodes_Size(), Lock_set);

        if(Entropy_Nodes->at(ele.node_id).tier == ele.next_tier){
            Max_Profit = 0;
        }
        else{
            Max_Profit = ele.profit;
            block_id = ele.node_id;
            tier_index = ele.next_tier;            
        }*/
        
        if(Max_Profit > 0){
            //cout<<"Max_Profit = "<<to_string(Max_Profit)<<endl;
            place_block_at_tier(block_id, tier_index);
            //cout<<"block_id = "<<to_string(block_id)<<"\t placed at = "<<to_string(tier_index)<<endl;
            Lock_set.insert(block_id);
            if(Lock_Queue.size() < Lock_Queue_size){
                Lock_Queue.push(block_id);
            }
            else{
                //resettle_node_in_Heap(this->Max_Heap, Lock_Queue.front(), HG->get_Nodes_Size());
                Lock_set.erase(Lock_Queue.front());
                Lock_Queue.pop();
                Lock_Queue.push(block_id);
            }
        }
        else{
            cout<<"Max_Profit = "<<to_string(Max_Profit)<<endl;
            cout<<"Returning MFM. rep = "<<to_string(i)<<endl;
            return;
        }
    }
    cout<<"Returning MFM. rep = "<<to_string(i)<<endl;
}

void Entropy_HyperGraph::Print_net_entropies(string file){
    ofstream ofs;
    ofs.open(file, ofstream::out);
    string write_string;
    initialize_Entropy();
    for(uint32_t i = 0; i < HG->get_Nets_Size(); i++){
        write_string = "n"+to_string(i+1)+"\t entropy = "+to_string(Entropy_Nets->at(i).Current_Entropy)+"\n";
        ofs.write(write_string.c_str(), strlen(write_string.c_str()));
    }
    double thw = 0;
    for (uint32_t i = 0; i < HG->get_Nets_Size(); i++)
    {
        double* Tier_sum = (double*)calloc(No_of_Tiers, sizeof(double));
        vector<uint32_t> connected_nodes = HG->get_connected_nodes_of(i);
        for(uint32_t j = 0; j < connected_nodes.size(); j++){
            Tier_sum[Entropy_Nodes->at(connected_nodes[j]).tier] += Entropy_Nodes->at(connected_nodes[j]).size;
        }
        for(uint8_t k = 0; k < No_of_Tiers; k++){
            thw += Tier_sum[k]*Tier_sum[k];
        }
    }
    write_string = "\nTHW = "+to_string(thw);
    ofs.write(write_string.c_str(), strlen(write_string.c_str()));
    ofs.close();
}

double Entropy_HyperGraph::Calculate_THW(){
    double thw = 0;
    double* Tier_sum = (double*)calloc(No_of_Tiers, sizeof(double));
    for (uint32_t i = 0; i < HG->get_Nets_Size(); i++){
        vector<uint32_t> connected_nodes = HG->get_connected_nodes_of(i);
        //cout<<"Net_id = "<<to_string(i)<<"\t degree = "<<to_string(connected_nodes.size())<<endl;
        for(uint32_t j = 0; j < connected_nodes.size(); j++){
            //cout<<"Net_id = "<<to_string(i)<<"\t block_id = "<<to_string(connected_nodes[j])<<endl;
            Tier_sum[Entropy_Nodes->at(connected_nodes[j]).tier] += Entropy_Nodes->at(connected_nodes[j]).size;
        }
        for(uint8_t k = 0; k < No_of_Tiers; k++){
            thw += Tier_sum[k]*Tier_sum[k];
            Tier_sum[k] = 0;
        }
    }
    free(Tier_sum);
    //cout<<"Total net = "<<to_string(HG->get_Nets_Size())<<endl;
    return thw;
}

void Entropy_HyperGraph::Area_Coverage(float allowed_percentage){

    double avg_area = Total_area / No_of_Tiers;
    double max_area = avg_area*(1 + allowed_percentage);
    double min_area = avg_area*(1 - allowed_percentage);
    double* Area = (double*)calloc(No_of_Tiers,sizeof(double));
    for (uint32_t i = 0; i < HG->get_Nodes_Size(); i++){
        Area[Entropy_Nodes->at(i).tier] += Block_Nodes->at(i).height*Block_Nodes->at(i).width;
    }
    //cout<<"Area calculation done"<<endl;
    for(uint8_t i = 0; i < No_of_Tiers; i++){
        if(Area[i] > max_area){
            //cout<<"calling max_area_coverage for layer = "<<to_string(i)<<" done"<<endl;
            max_area_coverage(Area, max_area, i);
            //cout<<"max_area_coverage for layer = "<<to_string(i)<<" done"<<endl;
        }
    }
    //cout<<"Returned from max_area_coverage(..)"<<endl;
    for(uint8_t i = 0; i < No_of_Tiers; i++){
        if(Area[i] < min_area){
            min_area_coverage(Area, min_area, i);
        }
    }
    //cout<<"Returned from min_area_coverage(..)"<<endl;
    free(Area);
    //cout<<"Returning from Area_coverage(..)"<<endl;
}

void Entropy_HyperGraph::max_area_coverage(double* Area, double max_area, uint8_t tier_index){
    while(Area[tier_index] > max_area){
        double Max_profit = 0, profit;
        uint32_t block_id;
        uint8_t tier;
        float block_area;
        bool flag = true;
        for(uint32_t i = 0; i < HG->get_Nodes_Size(); i++){
            if(Entropy_Nodes->at(i).tier == tier_index){
                for (uint8_t k = 0; k < No_of_Tiers; k++){
                    if(k != tier_index){
                        profit = gain_at_tier(i, k) - Entropy_Nodes->at(i).Gain;
                        block_area = Block_Nodes->at(i).height*Block_Nodes->at(i).height;
                        if(flag && Area[k]+block_area <= max_area){
                            Max_profit = profit;
                            block_id = i;
                            tier = k;
                            flag = false;
                        }
                        else if(profit > Max_profit && Area[k]+block_area <= max_area){
                            Max_profit = profit;
                            block_id = i;
                            tier = k;
                        }
                    }
                }
            }
        }
        if(flag){
            return;
        }
        else{
            Area[tier] += Block_Nodes->at(block_id).height*Block_Nodes->at(block_id).width;
            Area[tier_index] -= Block_Nodes->at(block_id).height*Block_Nodes->at(block_id).width;
            place_block_at_tier(block_id, tier);
        }
    }
}

void Entropy_HyperGraph::min_area_coverage(double* Area, double min_area, uint8_t tier_index){
    while(Area[tier_index] < min_area){
        double Max_profit, profit;
        uint32_t block_id;
        float block_area;
        bool flag = true;
        for(uint32_t i = 0; i < HG->get_Nodes_Size(); i++){
            if(Entropy_Nodes->at(i).tier != tier_index){
                profit = gain_at_tier(i, tier_index) - Entropy_Nodes->at(i).Gain;
                block_area = Block_Nodes->at(i).height*Block_Nodes->at(i).height;
                if(flag && Area[Entropy_Nodes->at(i).tier]-block_area >= min_area){
                    Max_profit = profit;
                    block_id = i;
                    flag = false;
                }
                else if(profit > Max_profit && Area[Entropy_Nodes->at(i).tier]-block_area >= min_area){
                    Max_profit = profit;
                    block_id = i;
                }
            }
        }
        if(flag){
            return;
        }
        else{
            Area[Entropy_Nodes->at(block_id).tier] -= Block_Nodes->at(block_id).height*Block_Nodes->at(block_id).width;
            Area[tier_index] += Block_Nodes->at(block_id).height*Block_Nodes->at(block_id).width;
            place_block_at_tier(block_id, tier_index);
        }
    }
}

void Entropy_HyperGraph::Shuffled_Partition(int repetation){
    set<uint32_t> Locked_Set;
    double* WC_list = (double*)calloc(No_of_Tiers, sizeof(double));
    uint32_t* WC_index = (uint32_t*)calloc(No_of_Tiers, sizeof(uint32_t));
    bool* WC_flag = (bool*)calloc(No_of_Tiers, sizeof(bool));
    double wc;
    uint8_t h1, h2;
    for(int count = 0; count < repetation; count++){
        h1 = 0;
        h2 = 1;
        for(uint8_t k = 0; k < No_of_Tiers; k++){
            WC_flag[k] = true;
        }
        for (uint32_t i = 0; i < HG->get_Nodes_Size(); i++){
            if(Locked_Set.find(i) == Locked_Set.end()){
                wc = weighted_conectivity_at_tier_level(i);
                if(WC_flag[Entropy_Nodes->at(i).tier]){
                    WC_list[Entropy_Nodes->at(i).tier] = wc;
                    WC_index[Entropy_Nodes->at(i).tier] = i;
                }
                else if(WC_flag[Entropy_Nodes->at(i).tier] < wc){
                    WC_list[Entropy_Nodes->at(i).tier] = wc;
                    WC_index[Entropy_Nodes->at(i).tier] = i;
                }
            }
        }
        if(No_of_Tiers > 2){
            if(WC_list[h2] > WC_list[h1]){
                h1 = 1;
                h2 = 0;
            }
            for(uint8_t i = 2; i < No_of_Tiers; i++){
                if(WC_list[i] > WC_list[h1]){
                    h2 = h1;
                    h1 = i;
                }
                else if(WC_list[i] > WC_list[h2]){
                    h2 = i;
                }
            }
        }
        place_block_at_tier(WC_index[h1], h2);
        place_block_at_tier(WC_index[h2], h1);
        Locked_Set.insert(WC_index[h1]);
        Locked_Set.insert(WC_index[h2]);
    }
}

uint32_t Entropy_HyperGraph::blocks_size(){
    return HG->get_Nodes_Size();
}

void Entropy_HyperGraph::load_Partition(){
    for(uint32_t i = 0; i < HG->get_Nodes_Size(); i++){
        Entropy_Nodes->at(i).tier = Partition[i];
        Entropy_Nodes->at(i).Gain = 0;
    }
    //initialize_Entropy();
}

bool Entropy_HyperGraph::Keep_ledger(uint32_t count){
    double curr_THW = Calculate_THW();
    if(count == 0){
        if(Partition != 0){
            cout<<"   Partition Freed"<<endl;
            free(Partition);
        }
        Partition = (uint8_t*)calloc(HG->get_Nodes_Size(), sizeof(uint8_t));
        THW = curr_THW;
        for(uint32_t i = 0; i < HG->get_Nodes_Size(); i++){
            Partition[i] = Entropy_Nodes->at(i).tier;
        }
    }
    else if(THW == curr_THW)
    {
        return false;
    }
    else if(THW > curr_THW){
        for(uint32_t i = 0; i < HG->get_Nodes_Size(); i++){
            Partition[i] = Entropy_Nodes->at(i).tier;
        }
        THW = curr_THW;
    }
    return true;
}

double Entropy_HyperGraph::Transition(double THW, double* Size_list,uint32_t block_id){
    const vector<uint32_t> connected_nets = HG->get_connected_nets_of(block_id);
    if(Entropy_Nodes->at(block_id).tier == 0){
        for(uint32_t i = 0; i < connected_nets.size(); i++){
            THW -= Size_list[Entropy_Nets->at(connected_nets[i]).id*No_of_Tiers + 0]*Size_list[Entropy_Nets->at(connected_nets[i]).id*No_of_Tiers + 0];
            Size_list[Entropy_Nets->at(connected_nets[i]).id*No_of_Tiers + 0] -= Entropy_Nodes->at(block_id).size;
            THW += Size_list[Entropy_Nets->at(connected_nets[i]).id*No_of_Tiers + 0]*Size_list[Entropy_Nets->at(connected_nets[i]).id*No_of_Tiers + 0];
            THW -= Size_list[Entropy_Nets->at(connected_nets[i]).id*No_of_Tiers + 1]*Size_list[Entropy_Nets->at(connected_nets[i]).id*No_of_Tiers + 1];
            Size_list[Entropy_Nets->at(connected_nets[i]).id*No_of_Tiers + 1] += Entropy_Nodes->at(block_id).size;
            THW += Size_list[Entropy_Nets->at(connected_nets[i]).id*No_of_Tiers + 1]*Size_list[Entropy_Nets->at(connected_nets[i]).id*No_of_Tiers + 1];
        }
        Entropy_Nodes->at(block_id).tier = 1;
    }
    else{
        for(uint32_t i = 0; i < connected_nets.size(); i++){
            THW -= Size_list[Entropy_Nets->at(connected_nets[i]).id*No_of_Tiers + 0]*Size_list[Entropy_Nets->at(connected_nets[i]).id*No_of_Tiers + 0];
            Size_list[Entropy_Nets->at(connected_nets[i]).id*No_of_Tiers + 0] += Entropy_Nodes->at(block_id).size;
            THW += Size_list[Entropy_Nets->at(connected_nets[i]).id*No_of_Tiers + 0]*Size_list[Entropy_Nets->at(connected_nets[i]).id*No_of_Tiers + 0];
            THW -= Size_list[Entropy_Nets->at(connected_nets[i]).id*No_of_Tiers + 1]*Size_list[Entropy_Nets->at(connected_nets[i]).id*No_of_Tiers + 1];
            Size_list[Entropy_Nets->at(connected_nets[i]).id*No_of_Tiers + 1] -= Entropy_Nodes->at(block_id).size;
            THW += Size_list[Entropy_Nets->at(connected_nets[i]).id*No_of_Tiers + 1]*Size_list[Entropy_Nets->at(connected_nets[i]).id*No_of_Tiers + 1];
        }
        Entropy_Nodes->at(block_id).tier = 0;
    }
    return THW;
}

void Entropy_HyperGraph::Simulated_Annelation(){
    srand(time(0));
    cout<<"RAND_MAX = "<<to_string(RAND_MAX)<<endl;
    float Tem = 60000;
    double last_thw, cur_thw;
    double prob;
    uint8_t tier_index;
    uint32_t block_id;
    cout<<"Total nodes = "<<to_string(HG->get_Nodes_Size())<<endl;
    
    for(uint32_t i = 0; i < HG->get_Nodes_Size(); i++){
        //cout<<"Inside Initial placement"<<endl;
        tier_index = abs(rand())%2;
        Entropy_Nodes->at(i).tier = tier_index;
        //cout<<"placing block = "+to_string(i)<<"\t at tier = "<<to_string(tier_index)<<endl;
    }

    double* Size_list = (double*)calloc(HG->get_Nets_Size()*No_of_Tiers, sizeof(double));
    for(uint32_t i = 0; i < HG->get_Nets_Size(); i++){
        const vector<uint32_t> connected_nodes = HG->get_connected_nodes_of(i);
        if(Size_list[i*No_of_Tiers + 0] != 0){
            cout<<"Size list not initialized to 0."<<endl;
            exit(0);
        }
        for(uint32_t k = 0; k < connected_nodes.size(); k++){
            Size_list[i*No_of_Tiers + Entropy_Nodes->at(connected_nodes[k]).tier] += Entropy_Nodes->at(connected_nodes[k]).size;
        }
    }

    last_thw = Calculate_THW();
    for(uint32_t cooling_step = 0; cooling_step < 500; cooling_step++){
        cout<<"Cooling Step _no = "<<to_string(cooling_step)<<endl;
        Tem = 0.9*Tem;
        for(uint32_t step = 0; step < 20000; step++){
            //cout<<"Step no = "<<to_string(step)<<endl;
            block_id = abs(rand())%HG->get_Nodes_Size(); 
            cur_thw = Transition(last_thw, Size_list, block_id);
            if(cur_thw > last_thw){
                prob = (double)abs(rand())/RAND_MAX;
                if((double)exp((last_thw - cur_thw)/Tem) >= prob){
                    last_thw = Transition(cur_thw, Size_list, block_id);
                }
                else{
                    last_thw = cur_thw;
                }
                
            }
            else{
                last_thw = cur_thw;
            }
        }
    }
}

uint64_t Entropy_HyperGraph::Calculate_MIV(){
    uint64_t MIV = 0;
    uint8_t top_tier, bottom_tier;
    for(uint32_t i = 0; i < HG->get_Nets_Size(); i++){
        top_tier = 0;
        bottom_tier = No_of_Tiers - 1;
        const vector<uint32_t> connected_nodes = HG->get_connected_nodes_of(i);
        for(uint32_t j = 0; j < connected_nodes.size(); j++){
            if(top_tier == (No_of_Tiers - 1) && bottom_tier == 0){
                break;
            }
            if(Entropy_Nodes->at(connected_nodes[j]).tier > top_tier){
                top_tier = Entropy_Nodes->at(connected_nodes[j]).tier;
            }
            if(Entropy_Nodes->at(connected_nodes[j]).tier < bottom_tier){
                bottom_tier = Entropy_Nodes->at(connected_nodes[j]).tier;
            }
        }
        MIV += (top_tier - bottom_tier);
    }
    return MIV;
}

void Heap_element_swap(Heap_t* Max_Heap, uint32_t first_ele_index, uint32_t second_ele_index){
    Heap_t tem;

    uint32_t first_id, second_id;

    first_id = Max_Heap[first_ele_index].node_id;
    second_id = Max_Heap[second_ele_index].node_id;

    tem.next_tier = Max_Heap[second_ele_index].next_tier;
    tem.node_id = Max_Heap[second_ele_index].node_id;
    tem.profit = Max_Heap[second_ele_index].profit;
    //tem.heap_index = Max_Heap[second_ele_index].heap_index;

    Max_Heap[second_ele_index].next_tier = Max_Heap[first_ele_index].next_tier;
    Max_Heap[second_ele_index].node_id = Max_Heap[first_ele_index].node_id;
    Max_Heap[second_ele_index].profit = Max_Heap[first_ele_index].profit;
    //Max_Heap[second_ele_index].heap_index = Max_Heap[first_ele_index].heap_index;

    Max_Heap[first_ele_index].next_tier = tem.next_tier;
    Max_Heap[first_ele_index].node_id = tem.node_id;
    Max_Heap[first_ele_index].profit = tem.profit;
    //Max_Heap[first_ele_index].heap_index = tem.heap_index;

    Max_Heap[first_id].heap_index = second_ele_index;
    Max_Heap[second_id].heap_index = first_ele_index;
}

void Heap_Decrease_Key(Heap_t* Max_Heap, uint32_t node_index, uint32_t Heap_Size){

    if(node_index > Heap_Size/2){
        return;
    }

    double child_profit = Max_Heap[node_index].profit;
    uint32_t next_child = -1;
    
    uint32_t chlid = 2*node_index + 1;         // Left child
    if(chlid < Heap_Size){
        if(Max_Heap[chlid].profit > child_profit){
            child_profit = Max_Heap[chlid].profit;
            next_child = chlid;
        }
    }
    chlid = 2*node_index + 2;                // right chid
    if(chlid < Heap_Size){
        if(Max_Heap[chlid].profit > child_profit){
            child_profit = Max_Heap[chlid].profit;
            next_child = chlid;
        }
    }

    if(next_child != uint32_t(-1)){
        //swap
        Heap_element_swap(Max_Heap, node_index, next_child);
        

        // Further check for decrease
        Heap_Decrease_Key(Max_Heap, next_child, Heap_Size);
    }
}

void Heap_Increase_Key(Heap_t* Max_Heap, uint32_t node_index, uint32_t Heap_Size){
    if(node_index == 0){
        return;
    }

    uint32_t parent_index = (node_index - 1) / 2;

    if(Max_Heap[parent_index].profit < Max_Heap[node_index].profit){

        // swap
        Heap_element_swap(Max_Heap, node_index, parent_index);

        // Further check for Increase
        Heap_Increase_Key(Max_Heap, parent_index, Heap_Size);
    }
}

void Entropy_HyperGraph::resettle_node_in_Heap(Heap_t* Max_Heap, uint32_t node_id, uint32_t Heap_Size){
    uint32_t node_heap_index = Max_Heap[node_id].heap_index;

    if(Max_Heap[node_heap_index].node_id != node_id){
        cout<<"Heap node id mismatch"<<endl;
        exit(0);
    }

    // Change the profit and tier value
    double current_gain = this->Gain_List[node_id*No_of_Tiers + Entropy_Nodes->at(node_id).tier];

    double profit = -3;         // since profit can't be less that -1
    uint8_t next_tier = Entropy_Nodes->at(node_id).tier;
    for(uint8_t k = 0; k < No_of_Tiers; k++){
        double tem;
        if(k != Entropy_Nodes->at(node_id).tier){
            tem = this->Gain_List[node_id*No_of_Tiers + k] - current_gain;
            if(tem > profit){
                profit = tem;
                next_tier = k;
            }
        }
    }
    Max_Heap[node_heap_index].profit = profit;
    Max_Heap[node_heap_index].next_tier = next_tier;


    // Check for Increse key option

    if(node_heap_index != 0){

        uint32_t parent_index = (node_heap_index - 1) / 2;
        if(Max_Heap[parent_index].profit < Max_Heap[node_heap_index].profit){
            Heap_Increase_Key(Max_Heap, node_heap_index, Heap_Size);
            return;
        }
    }
    Heap_Decrease_Key(Max_Heap, node_heap_index, Heap_Size);
    return;
}

Heap_t get_Best_Element(Heap_t* Max_Heap, uint32_t Heap_Size, const set<uint32_t>& Lock_set){
    if(Lock_set.find(Max_Heap[0].node_id) == Lock_set.end()){
        Heap_t ret;
        ret.profit = Max_Heap[0].profit;
        ret.node_id = Max_Heap[0].node_id;
        ret.next_tier = Max_Heap[0].next_tier;

        return ret;
    }
    else{
        Heap_element_swap(Max_Heap, 0, Heap_Size-1);
        Heap_Decrease_Key(Max_Heap, 0, Heap_Size - 1);
        return get_Best_Element(Max_Heap, Heap_Size-1, Lock_set);
    }
    
}

void Entropy_HyperGraph::Initialize_Heap(){
    if(Max_Heap != 0){
        free(Max_Heap);
    }

    Max_Heap = (Heap_t*)calloc(HG->get_Nodes_Size(), sizeof(Heap_t));

    for(uint32_t i = 0; i < HG->get_Nodes_Size(); i++){
        double current_gain = this->Gain_List[i*No_of_Tiers + Entropy_Nodes->at(i).tier];

        double profit = -3;         // since profit can't be less that -1
        uint8_t next_tier = Entropy_Nodes->at(i).tier;
        for(uint8_t k = 0; k < No_of_Tiers; k++){
            double tem;
            if(k != Entropy_Nodes->at(i).tier){
                tem = this->Gain_List[i*No_of_Tiers + k] - current_gain;
                if(tem > profit){
                    profit = tem;
                    next_tier = k;
                }
            }
        }
        Max_Heap[i].profit = profit;
        Max_Heap[i].node_id = i;
        Max_Heap[i].next_tier = next_tier;
        Max_Heap[i].heap_index = i;
    }

    //Build Heap
    uint32_t Heap_Size = HG->get_Nodes_Size();
    for(uint32_t i = Heap_Size/2; i >= 0; i--){
        //cout<<"i = "<<to_string(i)<<endl;
        Heap_Decrease_Key(this->Max_Heap, i, Heap_Size);
        if(i == 0){
            break;
        }
    }
}