//
//  heatmap.cpp
//  heatmap
//
//  Created by Tobias Rohloff
//  Copyright (c) 2013 Tobias Rohloff. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <list>
#include <sstream>
#include <sys/time.h>
#ifdef _OPENMP
    #include <omp.h>
#endif
#include <cstdlib>

using namespace std;

// structs

struct thread_data {
	int i, round;
};

struct hotspot {
	int x, y, start, end;
};

struct coord {
	int x, y;
};

// global variables

bool coord_mode = false;
int width, height, rounds;

double** heatmap;
double** next_heatmap;
thread_data* td;

list<hotspot> hotspots;
list<coord> coords;

// function definition

void* heat(void* args);

// main

int main(int argc, const char * argv[])
{
    timeval start, end;
    gettimeofday(&start, 0);
    
    // read parameter
    
    if (argc == 6) {
		coord_mode = true;
	} else if (argc != 5) {
        fprintf(stderr, "wrong parameter\n");
		return EXIT_FAILURE;
    }
    width = atoi(argv[1]);
    height = atoi(argv[2]);
    rounds = atoi(argv[3]);
    rounds++;
    
    string value;
    ifstream file_hotspots(argv[4]);
    if (file_hotspots.is_open()) {
        getline(file_hotspots, value);
        while (getline(file_hotspots, value)) {
            string cell;
            stringstream line(value);
            hotspot new_hotspot;
            
            getline(line, cell, ',');
            new_hotspot.x = atoi(cell.c_str());
            
            getline(line, cell, ',');
            new_hotspot.y = atoi(cell.c_str());
            
            getline(line, cell, ',');
            new_hotspot.start = atoi(cell.c_str());
            
            getline(line, cell, ',');
            new_hotspot.end = atoi(cell.c_str());
            
            hotspots.push_back(new_hotspot);
        }
        
#ifdef DEBUG
        cout << "hotspots: " << hotspots.size() << endl;
#endif
        
        file_hotspots.close();
    }
    else
        cout << "Unable to open file " << argv[4] << endl;
    
    if (coord_mode) {
        ifstream file_coords(argv[5]);
        if (file_coords.is_open()) {
            getline(file_coords, value);
            while (getline(file_coords, value)) {
                string cell;
                stringstream line(value);
                coord new_coord;
                
                getline(line, cell, ',');
                new_coord.x = atoi(cell.c_str());
                
                getline(line, cell, ',');
                new_coord.y = atoi(cell.c_str());
                
                coords.push_back(new_coord);
            }
            
#ifdef DEBUG
            cout << "coords: " << coords.size() << endl;
#endif
            
            file_coords.close();
        }
    }
    
    // create maps
    
    heatmap = new double*[height];
    next_heatmap = new double*[height];

	for(int i = 0; i < height; i++) {
		heatmap[i] = new double[width];
        next_heatmap[i] = new double[width];
    }
    
    td = new thread_data[height];
    
    // start working
    
    for(int r = 0; r < rounds; r++) {
        
#ifdef DEBUG
        cout << "round " << r << endl;
#endif
        
        #pragma omp parallel for
        for(int i = 0; i < height; i++) {
            td[i].i = i;
            td[i].round = r;
            heat(&td[i]);
        }
        double** help = heatmap;
        heatmap = next_heatmap;
        next_heatmap = help;
        
#ifdef DEBUG
        for(int i = 0; i < height; i++) {
            for(int j = 0; j < width; j++) {
                if (heatmap[i][j] > 0.9)
                    cout << "X";
                else {
                    double output = heatmap[i][j];
                    output += 0.09;
                    output *= 10;
                    cout << (int) output;
                }
            }
            cout << "" << endl;
        }
#endif
        
    }
    
    // create output files
    
    ofstream output_file("output.txt");
    if (output_file.is_open()) {
        if (coord_mode) {
            list<coord>::iterator i;
            for (i = coords.begin(); i != coords.end(); ++i)
                output_file << heatmap[i->y][i->x] << endl;
        } else {
            for(int i = 0; i < height; i++) {
                for(int j = 0; j < width; j++) {
                    if (heatmap[i][j] > 0.9) {
                        output_file << "X";
                    } else {
                        double output = heatmap[i][j];
                        output += 0.09;
                        output *= 10;
                        output_file << (int) output;
                    }
                }
                output_file << "" << endl;
            }
        }
        output_file.close();
    }
    else
        cout << "Unable to open file" << endl;
    
    // measure runtime
    
    gettimeofday(&end, 0);
    double elapsed = (end.tv_sec - start.tv_sec) + ((end.tv_usec - start.tv_usec)/1000000.0);
    cout << "runtime: " << elapsed << " s" << endl;
    
    return EXIT_SUCCESS;
}

void* heat(void* args)
{
    thread_data* td = (thread_data*)args;
    double avg;
    bool is_hotpot;
    
    list<hotspot>::iterator i;
    
    for (int j = 0; j < width; j++) {
        is_hotpot = false;
        avg = 0;
        
        for (i = hotspots.begin(); i != hotspots.end(); ++i)
            if (i->x == j && i->y == td->i && i->start <= td->round && td->round < i->end)
                is_hotpot = true;
        
        if (is_hotpot)
            avg = 1;
        else {
            if (td->i != 0 && j != 0)
                avg += heatmap[td->i-1][j-1];
            if (td->i != 0)
                avg += heatmap[td->i-1][j];
            if (td->i != 0 && j != width - 1)
                avg += heatmap[td->i-1][j+1];
            if (j != 0)
                avg += heatmap[td->i][j-1];
            avg += heatmap[td->i][j];
            if (j != width - 1)
                avg += heatmap[td->i][j+1];
            if (td->i != height - 1 && j != 0)
                avg += heatmap[td->i+1][j-1];
            if (td->i != height - 1)
                avg += heatmap[td->i+1][j];
            if (td->i != height - 1 && j != width - 1)
                avg += heatmap[td->i+1][j+1];
            avg /= 9;
        }
        next_heatmap[td->i][j] = avg;
    }
    return NULL;
}
