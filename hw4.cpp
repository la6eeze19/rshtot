#include <iostream>
#include <random>
#include <queue>
#include <vector>
#include <math.h>

using namespace std;
// A class to represent a server
class Server {
public:
    explicit Server(int queueSize,double m) : queue_size(queueSize), m_(m), randGen(random_device{}()){}

    // Generate a service time for a request using an exponential distribution with parameter m

    double m_;  // The rate parameter of the exponential distribution
    mt19937 randGen;  // A random number generator
    // An exponential distribution with parameter m

    queue<double > q_events;

    int queue_size;

    long double GetServiceTime() {
        exponential_distribution<double> exp_dist_(m_);
        return exp_dist_(randGen);
    }
};

// A class to represent the load balancer
class LoadBalancer {
public:
    // Constructor
    LoadBalancer(double T, int N, const vector<double>& P, double lambda,
                 const vector<int>& Q, const vector<double>& M)
            : maxTime(T),

              numServers(N),

              probs(P),

              lambda_(lambda),

              serRate(M),

              randGen(random_device{}()),

              unif_dist_(0, 1)
    {
        // Initialize the servers with the specified service rates
        for (int i = 0; i < N; i++) {
            Server tmp(Q[i],M[i]);
            servers.push_back(tmp);
        }
    }


    int pickServer(){

        double p = unif_dist_(randGen);
        // Initialize server index to -1
        int server_idx = -1;
        // Initialize sum to 0
        double sum = 0;
        // Loop through the servers
        for (int i = 0; i < numServers; i++) {
            sum += probs[i];
            // If p is less than sum, set the server index to i and break out of the loop
            if (p < sum) {
                server_idx = i;
                break;
            }
        }


        return server_idx;

    }




    // Run the simulation
    void Simulate() {
        // Initialize time to 0
        double t = 0;
        // Loop until time exceeds T
        while (t < maxTime) {

            // if particles decay once per second on average,
            // how much time, in seconds, until the next one?


            // Generate the time interval until the next request arrives using the exponential distribution with parameter lambda
            exponential_distribution<double> dt(lambda_);

            // Update time
            t += dt(this->randGen);
            // If time exceeds T, break out of the loop
            if (t > maxTime) {
                break;
            }

            // Create a new event for the request with the service time set to t + wait_time and the event type set to SERVICE
            int server_idx = pickServer();
            ProcessEvent(t, server_idx);

        }


    }


    // Process an event
    void ProcessEvent(double arrivalTime, int server_idx){

        // Get a reference to the server
        Server& server = servers[server_idx];

        while(server.q_events.front() < arrivalTime && !server.q_events.empty()){
            server.q_events.pop();
        }

        if (server.q_events.size() < servers[server_idx].queue_size + 1) {
            this->requestsHandled++;

            // Generate a service time for the request using an exponential distribution with parameter M[server_idx] and add it's service time
            double service_time = server.GetServiceTime();  // according to mue of server
            TotalServiceTime += service_time;
            if(server.q_events.empty()){
                server.q_events.push(arrivalTime+service_time);
            }
            else{
                double last_req_time = server.q_events.back();
                server.q_events.push(last_req_time+service_time);

                this->totalWaitingTime+= (last_req_time-arrivalTime);
            }

        }
        else{
            this->requestsThrown++;
        }
    }

    // Get the number of served requests
    int GetNumServedRequests() const {
        return requestsHandled;
    }

    // Get the number of dropped requests
    int GetNumDroppedRequests() const {
        return requestsThrown;
    }

    // Get the finishing time of the last request
    double GetLastFinishTime() const {
        double last = 0;
        for(auto  server: servers){
            double tmp  = server.q_events.empty() ? -1 : server.q_events.back();
            last = std::max(last,tmp);
        }
        return last;
    }

    // Get the average waiting time of a request
    double GetAverageWaitTime() const {
        return totalWaitingTime / requestsHandled;
    }

    // Get the average service time of a request
    double GetAverageServiceTime() const {
        return TotalServiceTime / requestsHandled;
    }

    double maxTime;  // The overall run time

    int numServers;  // The number of servers

    vector<double> probs;  // The probabilities with which requests are sent to each server

    double lambda_;  // The parameter of the exponential distribution of the request arrival rate

    vector<double> serRate;  // The service rates of the servers

    mt19937 randGen;  // A random number generator

    uniform_real_distribution<double> unif_dist_;  // A uniform real distribution

//    exponential_distribution<double> exp_dist_;  // An exponential distribution with parameter lambda

    vector<Server> servers;  // The servers

//    priority_queue<Event, vector<Event>, EventComparator> events;  // A priority queue of events

    int requestsHandled = 0;  // The number of served requests
    int requestsThrown = 0;  // The number of dropped requests
    double totalWaitingTime = 0;  // The total waiting time of served requests
    double TotalServiceTime = 0;  // The total service time of served requests
};

int main(int argc, char *argv[]) {

    // saving the inputs
    string totalTime = string(argv[1]);

    //saving input 1 ehich is the total run time
    double T = stold(totalTime);

    //saving the number of servers
    long int N = stoi(argv[2]);

    vector<double> P;
    int j;

    //saving the propability in which load balancer moves a request to each one of the servers
    //starting from the 3'rd argument ending in the N+2 argument
    for(j=3;j<N+3;j++){
        string str_arg = string(argv[j]);
        long double currProbability = stold(str_arg);
        P.push_back(currProbability);
    }

    //saving the rate in which requests arrive
    string arrRateStr = string(argv[j++]);
    double lambda = stold(arrRateStr);

    vector<int> Q;
    //saving the queues capacities for each one of the servers
    //starting from the  N + 4  argument ending in the 2N + 3 argument
    for(;j<2*N+4;j++){
        string str_arg = string(argv[j]);
        int currCapacity = stoi(str_arg);
        Q.push_back(currCapacity);
    }

    //saving the rate of request handling of the servers
    // starting from 2N + 4 argument ending in the 3N + 3 argument
    vector<double> M;
    for(;j< 3*N+4;j++){
        string str_arg = string(argv[j]);
        double curr_service = stold(str_arg);
        M.push_back(curr_service);
    }

    // Create a load balancer
    LoadBalancer load_balancer(T, N, P, lambda, Q, M);

    // Run the simulation
    load_balancer.Simulate();

    // Process the events in the priority queue until it is empty

    // Print the results of the simulation
    cout << load_balancer.GetNumServedRequests() <<" "<< load_balancer.GetNumDroppedRequests() << " "
         << load_balancer.GetLastFinishTime() << " "<< load_balancer.GetAverageWaitTime() << " " <<
         load_balancer.GetAverageServiceTime() << endl;

    return 0;
}