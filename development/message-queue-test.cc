#include <iostream>

#include <utility> // std::pair
#include <tuple>   // std::tuple
#include <list>    // std::list

#include "mpi.h"

namespace MPILander {

    const int any_tag = MPI_ANY_TAG;
    const int any_src = MPI_ANY_SOURCE;

    namespace MessageQueue {

        // MPI message envelope are the arguments used for matching:
        // { int rank, int tag, MPI_Comm communicator }
        // but we can elide the communicator by associating
        // a message queue with every communicator.
        // Since this project only supports one process, we will omit the rank too.
        using Envelope = std::pair<int,int>; // rank, tag

        // MPI message payload is the rest of the arguments
        // to MPI send:
        // { void* buffer, size_t count, MPI_Datatype type }
        // We will use size_t to handle large-count support.
        using Contents = std::tuple<void*,size_t,MPI_Datatype>;

        using Message = std::pair<Envelope,Contents>;

        class Queue {

            private:
                std::list<Message> q;

                void print_message(const Message& msg, const std::string& context = "") const {
                    auto [envelope, contents] = msg;
                    auto [rank, tag] = envelope;
                    auto [buffer, count, datatype] = contents;
                    if (!context.empty()) {
                        std::cout << context << ": ";
                    }
                    std::cout << "envelope(" << rank << "," << tag << "), contents(" << buffer << "," << count << "," << datatype << ")\n";
                }

            public:

                Queue() {}
                ~Queue() {
                    if (!q.empty()) {
                        std::cerr << "Queue not empty when dtor called!" << std::endl;
                        for (auto m = q.cbegin() ; m != q.cend() ; m++) {
                            auto [mrank,mtag] = std::get<0>(*m);
                            std::cerr << "Not empty: rank=" << mrank << ", tag=" << mtag << "\n";
                        }
                    }
                }

                void Insert(const Message& m) { 
                    // insert at the back of the queue
                    q.push_back(m); 
                }

                void Insert(const Envelope& e, const Contents& c) { 
                    q.push_back(std::make_pair(e,c)); 
                }

                void PrintMessage(const Message& msg, const std::string& context = "") const {
                    print_message(msg, context);
                }

                bool Match(int src, int tag, Message & msg) {

                    // fast exit if the message queue is empty
                    if (q.empty()) {
                        return false;
                    }

                    // wildcards first...
                    if (src == any_src && tag == any_tag) {
                        auto m = q.cbegin();
                        auto [mrank,mtag] = std::get<0>(*m);
                        std::cout << "Wildcard match: rank=" << mrank << ", tag=" << mtag << "\n";
                        msg = *m;
                        //print_message(msg, "msg contains");
                        q.erase(m);
                        return true;
                    }
                    else if (src == any_src) {
                        for (auto m = q.cbegin() ; m != q.cend() ; m++) {
                            auto [mrank,mtag] = std::get<0>(*m);
                            if (mtag==tag) {
                                std::cout << "Match: rank=" << mrank << ", tag=" << mtag << "\n";
                                msg = *m;
                                //print_message(msg, "msg contains");
                                q.erase(m);
                                return true;
                            }
                        }
                    }
                    else if (tag == any_tag) {
                        for (auto m = q.cbegin() ; m != q.cend() ; m++) {
                            auto [mrank,mtag] = std::get<0>(*m);
                            if (mrank==src) {
                                std::cout << "Match: rank=" << mrank << ", tag=" << mtag << "\n";
                                msg = *m;
                                //print_message(msg, "msg contains");
                                q.erase(m);
                                return true;
                            }
                        }
                    }
                    else {
                        for (auto m = q.cbegin() ; m != q.cend() ; m++) {
                            auto [mrank,mtag] = std::get<0>(*m);
                            if (mrank==src && mtag==tag) {
                                std::cout << "Match: rank=" << mrank << ", tag=" << mtag << "\n";
                                msg = *m;
                                //print_message(msg, "msg contains");
                                q.erase(m);
                                return true;
                            }
                        }
                    }

                    return false;
                }

                Queue(const Queue&) = delete;   // disable copy ctor
                Queue(Queue&&) = delete;        // disable move ctor

                Queue& operator=(const Queue&) = delete;    // disable copy assignment
                Queue& operator=(Queue&&) = delete;         // disable move assignment
        };

    } // Message namespace

} // MPILander namespace

#include <cstdlib>

int main(int argc, char* argv[])
{
    std::cout << "MPILander Message Queue Test" << std::endl;

    int n = (argc>1) ? std::atoi(argv[1]) : 100;

    MPILander::MessageQueue::Queue q;

    char* buffer = new char[100];

    // {{rank,tag}, {buffer,count,datatype}}
    q.Insert({{0,0},{NULL,0,0}});
    q.Insert({{0,1},{NULL,0,0}});
    q.Insert({{0,2},{NULL,0,0}});
    q.Insert({{0,3},{NULL,0,0}});
    q.Insert({{0,4},{NULL,0,0}});
    q.Insert({{0,5},{NULL,0,0}});
    q.Insert({{0,6},{NULL,0,0}});
    q.Insert({{0,7},{NULL,0,0}});
    q.Insert({{0,0},{&buffer[0],1,0}});
    q.Insert({{0,0},{&buffer[1],2,0}});
    q.Insert({{0,0},{&buffer[3],3,0}});
    q.Insert({{0,0},{&buffer[6],4,0}});
    q.Insert({{0,0},{&buffer[10],5,0}});
    q.Insert({{0,0},{&buffer[15],6,0}});
    q.Insert({{0,0},{&buffer[21],7,0}});

    MPILander::MessageQueue::Message msgout;
    
    while ( q.Match(MPILander::any_src, MPILander::any_tag, msgout) == true) {
        q.PrintMessage(msgout, "msgout");
    }

    std::cout << "The End" << std::endl;
    
    delete[] buffer;

    return 0;
}
