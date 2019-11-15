#include "HyperGraph_Entropy_Partitioner.h"
int main(int argc, char** argv){
    if(argc == 1 || (Strcmpi(argv[1], "-help") == 0) || (Strcmpi(argv[1], "-h") ==0))
    {
        cout<<"Syntax to run:"<<endl;
        cout<<"./exe [-blocks filename] [-nets filename] [-benchmark bechmarkname]"<<endl;
        exit(0);
    }
    int blockfile_index = 0, netfile_index = 0, benchname_index = 0, ip_index = 0, SA = 0;
    for(int i =1 ; i < argc ; i ++){
        if(Strcmpi(argv[i], "-blocks") == 0){
            i += 1;
            blockfile_index = i;
        }
        else if(Strcmpi(argv[i], "-nets") == 0){
            i += 1;
            netfile_index = i;
        }
        else if(Strcmpi(argv[i], "-benchmark") == 0){
            i += 1;
            benchname_index = i;
        }
        else if(Strcmpi(argv[i], "-ipname") == 0){
            i += 1;
            ip_index = i;
        }
        else if(Strcmpi(argv[i], "-SA") == 0){
            SA = 1;
        }
        else
        {
            cout<<"Syntax to run:"<<endl;
            cout<<"./exe [-blocks filename] [-nets filename] [-benchmark bechmarkname]"<<endl;
            exit(0);     
        }
        
    }
    try{
        HyperGraph_Entropy_Partioner G(argv[blockfile_index], argv[netfile_index], argv[benchname_index]);
        if(SA == 1){
            G.init_simulated();
        }
        else if(ip_index != 0){
            G.init(3, string(argv[ip_index]));
        }
        else{
            G.init(3);
        }

        
    }
    catch(HyperGraph_Exception e){
        cout<<e.what()<<endl;
    }
    catch(exception e){
        cout<<"Library exception occured = "<<e.what()<<endl;
        cout << "Type:    " << typeid(e).name() << "\n";
    }
    /*catch(...){
        cout<<"Some undefined exception occured"<<endl;
    }*/
    return 0;
}