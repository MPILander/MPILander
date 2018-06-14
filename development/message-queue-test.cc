#include <iostream>

#include <utility> // std::pair
#include <tuple>   // std::tuple
#include <list>    // std::list

typedef int MPI_Datatype;

namespace MPILander {

    namespace Message {

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

                Queue();
                ~Queue();

                void Insert(const Message& m) { q.push_back(m); }
                void Match(const Message& m) {
                    auto [mrank,mtag] = std::get<0>(m);
                    for (const auto& i : q) {
                        auto [irank,itag] = std::get<0>(i);
                        if (irank==mrank && itag==mtag) {
                            std::cout << "Match: rank=" << mrank << ", tag=" << mtag << "\n";
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






    std::cout << "The End" << std::endl;

    return 0;
}
