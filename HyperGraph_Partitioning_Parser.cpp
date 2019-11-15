#include "HyperGraph_Partitioning_Parser.h"

HyperGraph_Partitioning_Parser::HyperGraph_Partitioning_Parser
(const char* blocks_file,const char* nets_file, const char* benchmark) 
: blocks_file(blocks_file), nets_file(nets_file){
    //char benchmark_lower[20];
    //strcpy(benchmark_lower, benchmark);
    cout<<"HyperGraph_Partitioning_Parser(const char* blocks_file, const char* nets_file, const char* benchmark) called"<<endl;
    benchmark_type = -1;

    if(Strcmpi(benchmark, "ispd") == 0){
        benchmark_type = 0;
    }

    else if(Strcmpi(benchmark, "opencore_soft") == 0)    //"ami33"
    {
        cout<<"soft benchmark found"<<endl;
        benchmark_type = 1;
    }

    else if(Strcmpi(benchmark, "opencore_hard") == 0)    //"ami49, n*"
    {
        cout<<"hard benchmark found"<<endl;
        benchmark_type = 2;
    }
    Block_Nodes = new vector<block_node>;
    Node_id_to_Name = new vector<string>;
    HG = new HyperGraph();
    cout<<"HyperGraph created"<<endl;
}
HyperGraph_Partitioning_Parser::~HyperGraph_Partitioning_Parser(){
    //cout<<"Destructing Parser"<<endl;
    delete HG;
    delete Block_Nodes;
    delete Node_id_to_Name;
}

void HyperGraph_Partitioning_Parser::build_HyperGraph(){
    //cout<<"Build HyperGraph Called"<<endl;
    Tok = new Tokenizer(blocks_file, delimeters, end_char);
    switch (benchmark_type)
    {
    case 0:
        parse_ispd_blocks();
        delete Tok;
        Tok = new Tokenizer(nets_file, delimeters, end_char);
        parse_ispd_nets();
        delete Tok;
        break;

    case 1:
        parse_opencore_soft_blocks();
        delete Tok;
        Tok = new Tokenizer(nets_file, delimeters, end_char);
        parse_opencore_soft_nets();
        delete Tok;
        break;
    
    case 2:
        parse_opencore_hard_blocks();
        delete Tok;
        Tok = new Tokenizer(nets_file, delimeters, end_char);
        parse_opencore_hard_nets();
        delete Tok;
        break;
    
    default:
        throw HyperGraph_Exception("Benchmark not supported");
        break;
    }
    cout<<"no of nodes = "<<to_string(HG->get_Nodes_Size())<<endl;
    cout<<"No of nets = "<<to_string(HG->get_Nets_Size())<<endl;
    HG->build_connectivity();
    cout<<"Graph built"<<endl;
}

void HyperGraph_Partitioning_Parser::parse_opencore_soft_blocks(){
    //cout<<"Parsing Softblock"<<endl;
    char* tok = Tok->get_token();
    char node_name[TOK_SIZE];
    while (tok != 0)
    {
        if(strlen(tok) > 2 && tok[0] =='b' && tok[1] =='k'){
            strcpy(node_name, tok);
            Tok->get_token();    // softrectangular
            float area = atof(Tok->get_token());
            block_node tem;
            tem.id = HG->add_node(node_name);
            tem.height = tem.width = sqrt(area);
            Block_Nodes->push_back(tem);
            Node_id_to_Name->push_back(string(node_name));
        }
        tok = Tok->get_token();
    }
    
}

void HyperGraph_Partitioning_Parser::parse_opencore_soft_nets(){
    //cout<<"Parsing soft_nets"<<endl;
    char* tok = Tok->get_token();
    uint32_t net_count = 0;
    uint32_t net_id;
    while (tok != 0)
    {
        if(strcmp("NetDegree", tok) == 0){
            net_id = HG->add_net(to_string(net_count));
            if(net_id != net_count){
                throw HyperGraph_Exception("net_id mismatch");
            }
            net_count += 1;
        }
        if(net_count > 0 && tok[0] =='b' && tok[1] =='k'){
            HG->connect_net(net_id, string(tok));
        }
        tok = Tok->get_token();
    }
}

void HyperGraph_Partitioning_Parser::parse_opencore_hard_blocks(){
    //cout<<"Parsing Hardblock"<<endl;
    char* tok = Tok->get_token();
    char node_name[TOK_SIZE];
    while (tok != 0)
    {
        if(strlen(tok) > 2 && (tok[0] =='M' || (tok[0] == 's' && tok[1] == 'b'))){
            strcpy(node_name, tok);
            Tok->get_token();    // hardrectilinear
            for(int i = 0; i < 5; i++)
            {
                Tok->get_token();       // co-ordinate values
            }
            block_node tem;
            tem.id = HG->add_node(node_name);
            tem.height = atof(Tok->get_token());
            tem.width = atof(Tok->get_token());
            Block_Nodes->push_back(tem);
            Node_id_to_Name->push_back(string(node_name));
        }
        tok = Tok->get_token();
    }

}

void HyperGraph_Partitioning_Parser::parse_opencore_hard_nets(){
    //cout<<"Parsing hard_nets"<<endl;
    char* tok = Tok->get_token();
    uint32_t net_count = 0;
    uint32_t net_id;
    while (tok != 0)
    {
        if(strcmp("NetDegree", tok) == 0){
            net_id = HG->add_net(to_string(net_count));
            if(net_id != net_count){
                throw HyperGraph_Exception("net_id mismatch");
            }
            net_count += 1;
        }
        if(net_count > 0 && (tok[0] =='M' || (tok[0] == 's' && tok[1] == 'b'))){
            HG->connect_net(net_id, string(tok));
        }
        tok = Tok->get_token();
    }

}

void HyperGraph_Partitioning_Parser::parse_ispd_blocks(){
    //cout<<"Parsing ISPD_block"<<endl;
    char* tok = Tok->get_token();
    char node_name[TOK_SIZE];
    while (tok != 0)
    {
        if(strlen(tok) > 2 && tok[0] =='b' && tok[1] =='k'){
            strcpy(node_name, tok);
            Tok->get_token();    // softrectangular
            float area = atof(Tok->get_token());
            block_node tem;
            tem.id = HG->add_node(node_name);
            tem.height = tem.width = sqrt(area);
            Block_Nodes->push_back(tem);
            Node_id_to_Name->push_back(string(node_name));
        }
        tok = Tok->get_token();
    }

}

void HyperGraph_Partitioning_Parser::parse_ispd_nets(){
    //cout<<"Parsing ISPD_nets"<<endl;
    char* tok = Tok->get_token();
    uint32_t net_count = 0;
    uint32_t net_id;
    while (tok != 0)
    {
        if(strcmp("NetDegree", tok) == 0){
            net_id = HG->add_net(to_string(net_count));
            if(net_id != net_count){
                throw HyperGraph_Exception("net_id mismatch");
            }
            net_count += 1;
        }
        if(net_count > 0 && tok[0] =='b' && tok[1] =='k'){
            HG->connect_net(net_id, string(tok));
        }
        tok = Tok->get_token();
    }
}

void HyperGraph_Partitioning_Parser::print_net_connections(){
    int S = HG->get_Nets_Size();
    for(int i = 0; i < S; i++){
        const vector<uint32_t> V = HG->get_connected_nodes_of(i);
        cout<<"Net id = "<<i<<endl;
        for(int j = 0; j < V.size(); j++){
            cout<<"Block = "<<Node_id_to_Name->at(V[j])<<endl;
        }
    }
}

void HyperGraph_Partitioning_Parser::print_block_adjacency(){
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
