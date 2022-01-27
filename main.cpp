// Remember to add comments to your code

#include <iomanip>
#include <chrono>
#include <string>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <algorithm>
#include "pagetable.h"

// Check if an integer is power of 2
bool isPowerOfTwo(unsigned int x)
{
	/* First x in the below expression is for the case when x is 0 */
	return x && (!(x & (x - 1)));
}

int main(int argc, char* argv[]) {
	//Print basic information about the program
	std::cout << "=================================================================" << std::endl;
	std::cout << "CS 433 Programming assignment 5" << std::endl;
	std::cout << "Authors: Jake Tremblay, Danny Ha, Brian Richie" << std::endl;
	std::cout << "Date: 12/1/2021" << std::endl;
	std::cout << "Course: CS433 (Operating Systems)" << std::endl;
	std::cout << "Description : Program to simulate different page replacement algorithms" << std::endl;
	std::cout << "=================================================================\n" << std::endl;

	if (argc < 3) {
		// user does not enter enough parameters
		std::cout << "You have entered too few parameters to run the program.  You must enter" << std::endl
			<< "two command-line arguments:" << std::endl
			<< " - page size (in bytes): between 256 and 8192, inclusive" << std::endl
			<< " - physical memory size (in megabytes): between 4 and 64, inclusive" << std::endl;
		exit(1);
	}

	// Page size and Physical memory size 
	// Their values should be read from command-line arguments, and always a power of 2
	unsigned int page_size = atoi(argv[1]);

	if (!isPowerOfTwo(page_size)){
		std::cout << "You have entered an invalid parameter for page size (bytes)" << std::endl
			<< "  (must be an power of 2 between 256 and 8192, inclusive)." << std::endl;
		return 1;
	}

	unsigned int phys_mem_size = atoi(argv[2]) << 20; // convert from MB to bytes
	if (!isPowerOfTwo(phys_mem_size)){
		std::cout << "You have entered an invalid parameter for physical memory size (MB)" << std::endl
			<< "  (must be an even integer between 4 and 64, inclusive)." << std::endl;
		return 1;
	}

	// calculate number of pages and frames;
	int logic_mem_bits = 27;		// 27-bit logical memory (128 MB logical memory assumed by the assignment)
	int phys_mem_bits = std::log2(phys_mem_size);		// Num of bits for physical memory addresses, calculated from physical memory size, e.g. 24 bits for 16 MB memory
	int page_offset_bits = std::log2(page_size);				// Num of bits for page offset, calculated from page size, e.g. 12 bits for 4096 byte page
	// Number of pages in logical memory = 2^(logic_mem_bits - page_bit)
	int num_pages = 1 << (logic_mem_bits - page_offset_bits);
	// Number of free frames in physical memory = 2^(phys_mem_bits - page_offset_bits)
	int num_frames = 1 << (phys_mem_bits - page_offset_bits);

	std::cout << "Page size = " << page_size << " bytes" << std::endl;
	std::cout << "Physical Memory size = " << phys_mem_size << " bytes" << std::endl;
	std::cout << "Number of pages = " << num_pages << std::endl;
	std::cout << "Number of physical frames = " << num_frames << std::endl;


	PageTable ourTable;	
	
	// Test 1: Read and simulate the small list of logical addresses from the input file "small_refs.txt"
	std::cout << "\n================================Test 1==================================================\n";

	std::ifstream smallFile;
	smallFile.open("small_refs.txt", std::ios::in);
	if (!smallFile) {
		std::cout << "Error in file creation." << std::endl;
	}
	else {
		int input, frame = 0, pageFaults = 0, pageCount = 0, pageReplacements = 0;

		while (smallFile.eof() == false) {								//go through small reference file until end
			smallFile >> input;
			int pageNum = input / page_size;
			pageCount++;												//count up the references we get through

			PageEntry newPage(frame, false, false);						//create a new page entry using frame # we are currently on

			if (ourTable.pageMap.size() < num_frames) {
				if (ourTable.pageMap.find(pageNum) != ourTable.pageMap.end()) {	//if element is found, and the size of map isnt bigger than num frames
					auto it = ourTable.pageMap.find(pageNum);
					PageEntry target = it->second;
					target.dirty = false;
					std::cout << "Logical Memory address: " << input << " Page Number: " << pageNum << " Frame Number: " << target.frame_num << " Page Fault?: " << target.dirty << std::endl;
				}
				else {
					frame++;											//we only insert into a new frame if we actually have to insert, i believe
					pageFaults++;										//if we dont find the page in the map, page fault + 1
					newPage.dirty = true;									//set dirty bit to true so we know this has generated page fault
					newPage.valid = true;
					newPage.page_addr = pageNum;
					ourTable.pageMap.emplace(pageNum, newPage);				//ourTable is the page table, pageTable2 is a map<int pageNum, PageEntry>
					std::cout << "Logical Memory address: " << input << " Page Number: " << pageNum << " Frame Number: " << newPage.frame_num << " Page Fault?: " << newPage.dirty << std::endl;
				}
			}
			else {	//if map has grown to size of num_frames, we do replacements, this should not get hit in test 1
				pageReplacements++;
			}
			
		}
		smallFile.close();

		std::cout << "Number of references: " << pageCount << std::endl;
		std::cout << "Number of page faults: " << pageFaults << std::endl;
		std::cout << "Number of page replacements: " << pageReplacements << std::endl;

		ourTable.pageMap.clear();
		ourTable.pageVector.clear();
	}


	// Test 2: Read and simulate the large list of logical addresses from the input file "large_refs.txt"
	std::cout << "\n================================Test 2==================================================\n";
	std::cout << "****************Simulate FIFO replacement****************************" << std::endl;

	std::ifstream fifoFile;
	fifoFile.open("large_refs.txt", std::ios::in);
	if (!fifoFile) {
		std::cout << "Error in file creation." << std::endl;
	}
	else {
		auto start = std::chrono::high_resolution_clock::now();
		std::ios_base::sync_with_stdio(false);


		int input, frame = 0, pageFaults = 0, pageCount = 0, pageReplacements = 0;
		int i = 0;

		while (fifoFile.eof() == false) {									//go through large reference file until end
			fifoFile >> input;
			int pageNum = input / page_size;
			++pageCount;													//count up the references we get 

			PageEntry newPage(frame, false, false);							//create a new page entry using frame # we are currently on
			
			if (ourTable.pageMap.size() < num_frames) {							//if the map has free spots
				if (ourTable.pageMap.find(pageNum) == ourTable.pageMap.end()) {	//if no match in the table -> page fault
					frame++;
					pageFaults++;						

					newPage.dirty = true;										
					newPage.valid = true;
					newPage.page_addr = pageNum;
					ourTable.pageMap.emplace(pageNum, newPage);					//we just need to add element to set
					ourTable.pageQueue.push(newPage);
				}
			} else {															//map is now too big and we need FIFO replacements
				if (ourTable.pageMap.find(pageNum) == ourTable.pageMap.end()) {	//no match in table and table is full = FIFO 
					pageReplacements++;
					pageFaults++;

					PageEntry removed = ourTable.pageQueue.front();				//remove the front of the queue
					ourTable.pageQueue.pop();
					ourTable.pageMap.erase(removed.page_addr);					//erase the page address from the set

					newPage.dirty = true;
					newPage.valid = true;
					newPage.page_addr = pageNum;
					ourTable.pageMap.emplace(pageNum, newPage);					//add the new element to the set
					ourTable.pageQueue.push(newPage);
				}
			}
			
			
		}
		auto end = std::chrono::high_resolution_clock::now();
		double time_taken = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
		time_taken *= 1e-9;
		
		fifoFile.close();

		std::cout << "Number of references: " << pageCount << std::endl;
		std::cout << "Number of page faults: " << pageFaults << std::endl;
		std::cout << "Number of page replacements: " << pageReplacements << std::endl;
		std::cout << "Time taken by program is : " << std::fixed << time_taken << std::setprecision(4);
		std::cout << " sec" << std::endl;

		ourTable.pageMap.clear();												//clearing the pageMap since we reuse it next algo
	}

	std::cout << "****************Simulate Random replacement****************************" << std::endl;

	std::ifstream rFile;
	rFile.open("large_refs.txt", std::ios::in);
	if (!rFile) {
		std::cout << "Error in file creation." << std::endl;
	}
	else {
		auto start = std::chrono::high_resolution_clock::now();
		std::ios_base::sync_with_stdio(false);

		int input, frame = 0, pageFaults = 0, pageCount = 0, pageReplacements = 0;

		while (rFile.eof() == false) {											//go through large reference file until end
			rFile >> input;
			int pageNum = input / page_size;
			++pageCount;												

			PageEntry newPage(frame, false, false);						

			if (ourTable.pageMap.size() < num_frames) {		
				if (ourTable.pageMap.find(pageNum) == ourTable.pageMap.end()) {	//if no match in the table, page fault, add element to set
					frame++;
					pageFaults++;

					newPage.dirty = true;
					newPage.valid = true;
					newPage.page_addr = pageNum;
					ourTable.pageMap.emplace(pageNum, newPage);
					ourTable.pageVector.push_back(newPage);
				}
			}
			else {
				if (ourTable.pageMap.find(pageNum) == ourTable.pageMap.end()) { //if set is full and we cant find the match, need to random replace
					pageReplacements++;
					pageFaults++;

					srand(time(nullptr));										//choose a random victim page
					int r = rand() % num_frames;
					PageEntry target = ourTable.pageVector.at(r);				 

					ourTable.pageVector.at(r) = newPage;						//replace target
					ourTable.pageMap.erase(target.page_addr);					//erase target page from set

					ourTable.pageMap.emplace(pageNum, newPage);					//add new element
				}
			}
			

		}
		auto end = std::chrono::high_resolution_clock::now();
		double time_taken = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
		time_taken *= 1e-9;

		rFile.close();

		std::cout << "Number of references: " << pageCount << std::endl;
		std::cout << "Number of page faults: " << pageFaults << std::endl;
		std::cout << "Number of page replacements: " << pageReplacements << std::endl;
		std::cout << "Time taken by program is : " << std::fixed << time_taken << std::setprecision(4);
		std::cout << " sec" << std::endl;

		ourTable.pageMap.clear();
		ourTable.pageVector.clear();
	}

	std::cout << "****************Simulate LRU replacement****************************" << std::endl;

	std::ifstream lruFile;
	lruFile.open("large_refs.txt", std::ios::in);
	if (!lruFile) {
		std::cout << "Error in file creation." << std::endl;
	}
	else {
		auto start = std::chrono::high_resolution_clock::now();
		std::ios_base::sync_with_stdio(false);


		int input, frame = 0, pageFaults = 0, pageCount = 0, pageReplacements = 0;

		while (lruFile.eof() == false) {								//go through small reference file until end
			lruFile >> input;
			int pageNum = input / page_size;
			++pageCount;												//count up the references we get through

			PageEntry newPage(frame, false, false);						//create a new page entry using frame # we are currently on

			if (ourTable.pageMap.size() < num_frames) {    
				if (ourTable.pageMap.find(pageNum) == ourTable.pageMap.end()) {	//if no match in the table
					frame++;
					pageFaults++;

					newPage.dirty = true;
					newPage.valid = true;
					newPage.page_addr = pageNum;

					ourTable.pageMap.emplace(pageNum, newPage);
					ourTable.pageDeque.push_front(pageNum);		//front of deque is 'top' of stack			
				}
				else {											//page is present, we find its handle, erase it, and push it into the front position
					auto it = std::find(ourTable.pageDeque.begin(), ourTable.pageDeque.end(), pageNum);
					if (it != ourTable.pageDeque.end()) {
						ourTable.pageDeque.erase(it);
						ourTable.pageDeque.push_front(pageNum);
					}
				}
			}
			else {												//pageMap is now filled to capacity and we need to do lru replacements
				if (ourTable.pageMap.find(pageNum) == ourTable.pageMap.end()) {		//if page is not present, and our memory is full, we need to replace
					pageReplacements++;
					pageFaults++;

					PageEntry target;
					target.page_addr = ourTable.pageDeque.front();					//'top' of stack
					ourTable.pageDeque.pop_front();
					ourTable.pageMap.erase(target.page_addr);						//erase it from set

					ourTable.pageMap.emplace(pageNum, newPage);						//add the new element
					ourTable.pageDeque.push_front(pageNum);
				}
				else {											//page is present, find its handle and move it to top
					auto it = std::find(ourTable.pageDeque.begin(), ourTable.pageDeque.end(), pageNum);
					if (it != ourTable.pageDeque.end()) {
						ourTable.pageDeque.erase(it);
						ourTable.pageDeque.push_front(pageNum);
					}
				}
				
			}
		}

		auto end = std::chrono::high_resolution_clock::now();
		double time_taken = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
		time_taken *= 1e-9;

		lruFile.close();

		std::cout << "Number of references: " << pageCount << std::endl;
		std::cout << "Number of page faults: " << pageFaults << std::endl;
		std::cout << "Number of page replacements: " << pageReplacements << std::endl;
		std::cout << "Time taken by program is : " << std::fixed << time_taken << std::setprecision(4);
		std::cout << " sec" << std::endl;

		ourTable.pageMap.clear();
		ourTable.pageVector.clear();
	}

}
