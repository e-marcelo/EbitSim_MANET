

package br.larc.usp.client;

//
// Chokes/unchokes peers.
//
simple Choker
{
    parameters:
        // The infoHash of the swarm the Choker is serving
        int infoHash;
        // The rate of optimistic unchokes during leech state
        // (that means an optimistic unchoke occurs every 'n' rounds).
        int optimisticRoundRateInLeech = default(3);
        // The rate of optimistic unchokes during seed state
        int optimisticRoundRateInSeed = default(3);
        // Number of optimistic unchoked
        int numberOfOptimisticSlots = default(1);
        // Number of regular unchoked
        int numberOfRegularSlots = default(3);
        // The time between each unchoke round

        //double roundInterval @unit(s) = default(40s);
        //double roundInterval @unit(s) = default(10s);

//        double roundInterval @unit(s) = default(10s);
		double roundInterval @unit(s) = default(10s);

        // True if the Client have all pieces already.
        bool seeder;

        bool debugFlag = default(false);

    @display("i=block/cogwheel");
}
