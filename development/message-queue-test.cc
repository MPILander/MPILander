#include <iostream>

#include <utility> // std::pair
#include <tuple>   // std::tuple
#include <list>    // std::list

typedef int MPI_Datatype;

namespace MPILander {

    const int any_tag = -4096;
    const int any_src = -8192;

    namespace MessageQueue {

        // MPI message envelope are the arguments used for matching:
        // { int rank, int tag, MPI_Comm communicator }
        // but we can elide the communicator by associating
        // a message queue with every communicator.
        using Envelope = std::tuple<int,int>;

        // MPI message payload is the rest of the arguments
        // to MPI send:
        // { void* buffer, int count, MPI_Datatype type }
        using Contents = std::tuple<void*,int,MPI_Datatype>;

        using Message = std::pair<Envelope,Contents>;

        class Queue {

            private:
                std::list<Message> q;

            public:

                Queue() {};
                ~Queue() {};

                void Insert(const Message& m) { q.push_back(m); }
                void Match(int && src, int && tag) {

                    // fast exit if the message queue is empty
                    if (q.empty()) return;

                    // wildcards first...
                    if (src == any_src && tag == any_tag) {
                        auto m = q.front();
                        auto [mrank,mtag] = std::get<0>(m);
                        std::cout << "Match: rank=" << mrank << ", tag=" << mtag << "\n";
                    }
                    else if (src == any_src) {
                        for (const auto& m : q) {
                            auto [mrank,mtag] = std::get<0>(m);
                            if (mtag==tag) {
                                std::cout << "Match: rank=" << mrank << ", tag=" << mtag << "\n";
                            }
                        }
                    }
                    else if (tag == any_tag) {
                        for (const auto& m : q) {
                            auto [mrank,mtag] = std::get<0>(m);
                            if (mrank==src) {
                                std::cout << "Match: rank=" << mrank << ", tag=" << mtag << "\n";
                            }
                        }
                    }
                    else {
                        for (const auto& m : q) {
                            auto [mrank,mtag] = std::get<0>(m);
                            if (mrank==src && mtag==tag) {
                                std::cout << "Match: rank=" << mrank << ", tag=" << mtag << "\n";
                            }
                        }
                    }
                }

                void Insert(const Envelope& e, const Contents& c) { q.push_back(std::make_pair(e,c)); }
                //const Contents& Match(const Envelope& e) {
                //    for (
                //}

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
    MPILander::MessageQueue::Message m{{0,0},{NULL,0,0}};

    q.Insert(m);
    q.Match(0,0);

    std::cout << "The End" << std::endl;

    return 0;
}
