#include "ACO.h"

#include <cstdio>
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <limits>
#include <climits>
#include <random>

using namespace std;

ACO::ACO (int nAnts, int nCities, 
		double alpha, double beta, double q, double ro, double taumax,
		int initCity) {
	NUMBEROFANTS 	= nAnts;
	NUMBEROFCITIES 	= nCities;
	ALPHA 			= alpha;
	BETA 			= beta;
	Q 				= q;
	RO 				= ro;
	TAUMAX 			= taumax;
	INITIALCITY		= initCity;

	randoms = new Randoms (21);	
}
ACO::~ACO () {
	for(int i=0; i<NUMBEROFCITIES; i++) {
		delete [] GRAPH[i];
		delete [] CITIES[i];
		delete [] PHEROMONES[i];
		delete [] DELTAPHEROMONES[i];
		if(i < NUMBEROFCITIES - 1) {
			delete [] PROBS[i];
		}
	}
	delete [] GRAPH;
	delete [] CITIES;
	delete [] PHEROMONES;
	delete [] DELTAPHEROMONES;
	delete [] PROBS;
}


void ACO::init () {
	GRAPH 			= new int*[NUMBEROFCITIES];
	CITIES 			= new double*[NUMBEROFCITIES];
	PHEROMONES 		= new double*[NUMBEROFCITIES];
	DELTAPHEROMONES = new double*[NUMBEROFCITIES];
	PROBS 			= new double*[NUMBEROFCITIES-1];
	for(int i=0; i<NUMBEROFCITIES; i++) {
		GRAPH[i] 			= new int[NUMBEROFCITIES];
		CITIES[i] 			= new double[2];
		PHEROMONES[i] 		= new double[NUMBEROFCITIES];
		DELTAPHEROMONES[i] 	= new double[NUMBEROFCITIES];
		PROBS[i] 			= new double[2];
		for (int j=0; j<2; j++) {
			CITIES[i][j] = -1.0;
			PROBS[i][j]  = -1.0;
		}
		for (int j=0; j<NUMBEROFCITIES; j++) {
			GRAPH[i][j] 			= 0;
			PHEROMONES[i][j] 		= 0.0;
			DELTAPHEROMONES[i][j] 	= 0.0;
		}
	}	
	//Routes num_ants*num_cities
	ROUTES = new int*[NUMBEROFANTS];
	for (int i=0; i<NUMBEROFANTS; i++) {
		ROUTES[i] = new int[NUMBEROFCITIES];
		for (int j=0; j<NUMBEROFCITIES; j++) {
			ROUTES[i][j] = -1;
		}
	}
	
	BESTLENGTH = (double) INT_MAX;
	BESTROUTE  = new int[NUMBEROFCITIES];
	for (int i=0; i<NUMBEROFCITIES; i++) {
		BESTROUTE[i] = -1;	
	}
}

//Adds connection in graph and pheromone
void ACO::connectCITIES (int cityi, int cityj) {
	GRAPH[cityi][cityj] = 1;
	// PHEROMONES[cityi][cityj] = randoms -> Uniforme() * TAUMAX;
	PHEROMONES[cityi][cityj] = 1/(double)TAUMAX;	
	//Symmetric
	GRAPH[cityj][cityi] = 1;
	PHEROMONES[cityj][cityi] = PHEROMONES[cityi][cityj];
}
//Set x,y position. Will be removed
void ACO::setCITYPOSITION (int city, double x, double y) {
	CITIES[city][0] = x;
	CITIES[city][1] = y;
}
//Util
void ACO::printPHEROMONES () {	
	cout << " PHEROMONES: " << endl;
	cout << "  | ";
	for (int i=0; i<NUMBEROFCITIES; i++) {
		printf("%5d   ", i);
	}
	cout << endl << "- | ";
	for (int i=0; i<NUMBEROFCITIES; i++) {
		cout << "--------";
	}
	cout << endl;
	for (int i=0; i<NUMBEROFCITIES; i++) {
		cout << i << " | ";
		for (int j=0; j<NUMBEROFCITIES; j++) {
			if (i == j) {
				printf ("%5s   ", "x");
				continue;
			}
			if (exists(i, j)) {
				printf ("%7.3f ", PHEROMONES[i][j]);
			}
			else {
				if(PHEROMONES[i][j] == 0.0) {
					printf ("%5.0f   ", PHEROMONES[i][j]);
				}
				else {
					printf ("%7.3f ", PHEROMONES[i][j]);
				}
			}
		}
		cout << endl;
	}
	cout << endl;
}

//Util will be removed
double ACO::distance (int cityi, int cityj) {
	return (double) 
		sqrt (pow (CITIES[cityi][0] - CITIES[cityj][0], 2) + 
 			  pow (CITIES[cityi][1] - CITIES[cityj][1], 2));
}
//Checks if edge exists
bool ACO::exists (int cityi, int cityc) {
	return (GRAPH[cityi][cityc] == 1);
}
//Answers: Has ant k visited city c?
bool ACO::vizited (int antk, int c) {
	for (int l=0; l<NUMBEROFCITIES; l++) {
		if (ROUTES[antk][l] == -1) {
			break;
		}
		if (ROUTES[antk][l] == c) {
			return true;
		}
	}
	return false;
}

//Gives transition probability
double ACO::PHI (int cityi, int cityj, int antk) {
	double ETAij = (double) pow (1 / distance (cityi, cityj), BETA);
	double TAUij = (double) pow (PHEROMONES[cityi][cityj],   ALPHA);

	double sum = 0.0;
	for (int c=0; c<NUMBEROFCITIES; c++) {
		if (exists(cityi, c)) {
			if (!vizited(antk, c)) {
				double ETA = (double) pow (1 / distance (cityi, c), BETA);
				double TAU = (double) pow (PHEROMONES[cityi][c],   ALPHA);
				sum += ETA * TAU;
			}	
		}	
	}
	return (ETAij * TAUij) / sum;
}

//Gives length of tour
double ACO::length (int antk) {
	double sum = 0.0;
	for (int j=0; j<NUMBEROFCITIES-1; j++) {
		sum += distance (ROUTES[antk][j], ROUTES[antk][j+1]);	
	}
	return sum;
}
// Function to select the next city based on weird stuff
int ACO::city () {
	double xi = randoms -> Uniforme();
	cout<<"xi = "<<xi<<endl;
	int i = 0;
	double sum = PROBS[i][0];
	cout<<"Sum = "<<sum<<endl;
	while (sum < xi) {
		i++;
		sum += PROBS[i][0];
	}
	cout<<"final i = "<<i<<endl;
	cout<<"Correspondig final city = "<<(int) PROBS[i][1]<<endl;
	return (int) PROBS[i][1];
}

// Function to select the next city with highest probability. Count is max cities left
int ACO::city (int count) {
	double maxi=-1;
	int maxi_index = -1;
	for(int i=0;i<count;i++){
		double sum = PROBS[i][0];
		if (sum > maxi) {
			maxi = sum;
			maxi_index = i;
		}
	}
	return (int) PROBS[maxi_index][1];
}


//Try and create route for ant k
void ACO::route (int antk, int initial_city) {
	//Set initial city


	ROUTES[antk][0] = initial_city;
	//For all other cities
	for (int i=0; i<NUMBEROFCITIES-1; i++) {
		int cityi = ROUTES[antk][i];
		//Init visiting probabilities for other cities
		int count = 0;
		for (int c=0; c<NUMBEROFCITIES; c++) {
			//if current city continue.
			if (cityi == c) {
				continue;	
			}
			//if not visited city and edge exists
			if (exists (cityi, c)) {
				if (!vizited (antk, c)) {
					//Set visiting probability and city in PROBS array
					PROBS[count][0] = PHI (cityi, c, antk);
					PROBS[count][1] = (double) c;
					count++;
				}

			}
		}
		
		// deadlock
		if (0 == count) {
			return;
		}
		//Set next city in route as city 
		ROUTES[antk][i+1] = city(count);
	}
}

int ACO::valid (int antk, int iteration) {
	for(int i=0; i<NUMBEROFCITIES-1; i++) {
		//Routes are intially set to -1
		int cityi = ROUTES[antk][i];
		int cityj = ROUTES[antk][i+1];

		if (cityi < 0 || cityj < 0) {
			return -1;	
		}
		if (!exists(cityi, cityj)) {
			return -2;	
		}
		for (int j=0; j<i-1; j++) {
			if (ROUTES[antk][i] == ROUTES[antk][j]) {
				return -3;
			}	
		}
	}
	
	if (!exists (ROUTES[antk][0], ROUTES[antk][NUMBEROFCITIES-1])) {
		return -4;
	}
	
	return 0;
}

void ACO::printGRAPH () {
	cout << " GRAPH: " << endl;
	cout << "  | ";
	for( int i=0; i<NUMBEROFCITIES; i++) {
		cout << i << " ";
	}
	cout << endl << "- | ";
	for (int i=0; i<NUMBEROFCITIES; i++) {
		cout << "- ";
	}
	cout << endl;
	int count = 0;
	for (int i=0; i<NUMBEROFCITIES; i++) {
		cout << i << " | ";
		for (int j=0; j<NUMBEROFCITIES; j++) {
			if(i == j) {
				cout << "x ";	
			}
			else {
				cout << GRAPH[i][j] << " ";
				if(GRAPH[i][j]==1)
				{
					cout<<","<<distance(i,j)<<" ";
				}	
				else
				{
					cout<<",- ";
				}
			}
			if (GRAPH[i][j] == 1) {
				count++;	
			}
		}
		cout << endl;
	}
	cout << endl;
	cout << "Number of connections: " << count << endl << endl;
}
void ACO::printRESULTS () {
	BESTLENGTH += distance (BESTROUTE[NUMBEROFCITIES-1], BESTROUTE[0]);
	cout << " BEST ROUTE:" << endl;
	for (int i=0; i<NUMBEROFCITIES; i++) {
		cout << BESTROUTE[i] << " ";
	}
	cout<<endl;
	cout<<"num cities = "<<NUMBEROFCITIES<<endl;
	for (int i=0; i<NUMBEROFCITIES; i++) {
		int u = BESTROUTE[i];
		int v = BESTROUTE[(i+1)%NUMBEROFCITIES];
		cout << GRAPH[u][v] << " ";
	}
	cout << endl << "length: " << BESTLENGTH << endl;
	
	cout << endl << " IDEAL ROUTE:" << endl;
	cout << "0 7 6 2 4 5 1 3" << endl;
	cout << "length: 127.509" << endl;
}

void ACO::updatePHEROMONES () {
	for (int k=0; k<NUMBEROFANTS; k++) {
		double rlength = length(k);
		for (int r=0; r<NUMBEROFCITIES-1; r++) {
			int cityi = ROUTES[k][r];
			int cityj = ROUTES[k][r+1];
			DELTAPHEROMONES[cityi][cityj] += Q / rlength;
			DELTAPHEROMONES[cityj][cityi] += Q / rlength;
		}
	}
	for (int i=0; i<NUMBEROFCITIES; i++) {
		for (int j=0; j<NUMBEROFCITIES; j++) {
			PHEROMONES[i][j] = (1 - RO) * PHEROMONES[i][j] + DELTAPHEROMONES[i][j];
			DELTAPHEROMONES[i][j] = 0.0;
		}	
	}
}


void ACO::optimize (int ITERATIONS) {
	for (int iterations=1; iterations<=ITERATIONS; iterations++) {
		cout << flush;
		cout << "ITERATION " << iterations << " HAS STARTED!" << endl << endl;

		for (int k=0; k<NUMBEROFANTS; k++) {
			cout << " : ant " << k << " has been released!" << endl;
			    const int range_from  = 0;
			    const int range_to    = NUMBEROFCITIES-1; //Since range is inclusive
			    std::random_device                  rand_dev;
			    std::mt19937                        generator(rand_dev()); //Seed the generator
			    std::uniform_int_distribution<int>  distr(range_from, range_to);			
				int initial_city = distr(generator);

			while (0 != valid(k, iterations)) {
				cout << "  :: releasing ant " << k << " again!" << endl;
				for (int i=0; i<NUMBEROFCITIES; i++) {
					ROUTES[k][i] = -1;	
				}
				route(k, initial_city);
				// if(0 == valid(k, iterations))
				// {
				// 	break;
				// }
				initial_city = distr(generator);
			}
			
			for (int i=0; i<NUMBEROFCITIES; i++) {
				cout << ROUTES[k][i] << " ";	
			}
			cout << endl;
			
			cout << "  :: route done" << endl;
			double rlength = length(k);

			if (rlength < BESTLENGTH) {
				BESTLENGTH = rlength;
				for (int i=0; i<NUMBEROFCITIES; i++) {
					BESTROUTE[i] = ROUTES[k][i];
				}
			}
			cout << " : ant " << k << " has ended!" << endl;				
		}		

		cout << endl << "updating PHEROMONES . . .";
		updatePHEROMONES ();
		cout << " done!" << endl << endl;
		printPHEROMONES ();
		
		for (int i=0; i<NUMBEROFANTS; i++) {
			for (int j=0; j<NUMBEROFCITIES; j++) {
				ROUTES[i][j] = -1;
			}
		}

		cout << endl << "ITERATION " << iterations << " HAS ENDED!" << endl << endl;
	}
}
