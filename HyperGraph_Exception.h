#ifndef HYPERGRAPH_EXCEPTION_H
#define HYPERGRAPH_EXCEPTION_H
#include "Header.h"

#define ERROR_MSG_SIZE 100

class HyperGraph_Exception: std::exception{
    private:
        char err_msg[ERROR_MSG_SIZE];
    public:
        HyperGraph_Exception(const char* str){
            strncpy(err_msg, str, ERROR_MSG_SIZE);
        }
        const char* what() const noexcept{
            return err_msg;
        }

};


#endif