/*	hhs_vision_lib.c
	Hampton High School Botball Team
	Team #0141
	DC Region
	
	Adam Farabaugh - adam.farabaugh1@gmail.com
	Evan Wilson - emwilson02@gmail.com
	
	Change Log:
		Mar 23 2011		- Begun project - working with structures and arrays of them
		Mar 31 2011		- Removed 2nd dimension of data array - would have impeded filtering all channel data together
		Apr 03 2011		- just kidding - putting 2nd dimension back in because sorting will suck otherwise
		Apr 13 2011		- added some merging functions
		Apr 14 2011		- added a lot of sorting stuff
						- need to come up with a way to sort by distance from point
		Apr 15 2011		- added (float)dist attribute to structure so we can sort by dist from a point
	AS OF THIS LAST DATE EVERYTHING IS STILL UNCOMPILED/UNTESTED/UNDEBUGGED
		Apr 23 2011		- fixed a lot of stuff, especially sorting comparison functions, NOW IT COMPILES!
		Apr 24 2011		- changed all square roots to fastroot() and retyped distances to int
						- added 5th channel in vision_data_array for storing merged channels if desired
						- conditional printing to file and/or screen
						- adding warnings when some arguments don't comply
		Apr 25 2011		- fixed conditional printing
						- added timestamping to printing so you can just leave the USB in for a couple runs
		Jun 01 2011		- TIMESTAMPING NEEDS FIXED - LIBRARIES ARENT WORKING/DONT EXIST ON CBC (HELP?!?)
		Jul	06 2011		- Finished merge_channels() and started working on merge_by_bounds()
		Jul 15 2011		- Finished merge_by_bounds() algorithm, needs testing and hopefully some logic optimization
*/

// Types & Structures
typedef struct blob {
	int size, x, y;
	int bbl, bbr, bbt, bbb;
	int dist;
} BLOB;

// Preprocessor Switches
#define SCREENPRINT // Comment this out to disable printing of blob data to screen
//#define FILEPRINT // Comment this out to disable printing of blob data to file (requires a mountable USB drive)

// Macros
#define LARGE  1
#define SMALL -1

#define NUM_CHS 5
#define NUM_BLBS 10

#define vda vision_data_array

// Variables
BLOB vision_data_array[NUM_CHS][NUM_BLBS];
int _hhs_vision_initted = 0;

// Prototypes
	// Setup
		void init_hhs_vision();
	// Getting Data
		BLOB get_blob_data(int channel, int blob_index);
		void get_channel_blob_data(int channel);
		void get_all_blob_data();
	// Data Management
		void clear_blob_data(int i, int j);
		void clear_all_blob_data();
		void clean_vision_array(int channel);
		void print_vision_array();
		void print_one_channel(int ch);
	// Blob Filtering 
		void filter_by_size(int channel, int threshold, int less_or_greater);
		void filter_by_x(int channel, int threshold, int less_or_greater);
		void filter_by_y(int channel, int threshold, int less_or_greater);
		void filter_by_box(int channel, int x1, int y1, int x2, int y2, int internal_or_external);
	// Blob Merging
		void merge_blobs(int channel, int index1, int index2);
		void merge_by_distance(int channel, int threshold);
		void merge_by_bounds(int channel); // WORK ON THIS
		void merge_channels(int c1, int c2, int ctarget, int order);
	// Blob Sorting
		void sort_by_size(int channel, int small_or_large);
		void sort_by_x(int channel, int small_or_large);
		void sort_by_y(int channel, int small_or_large);
		void sort_by_point(int channel, int point_x, int point_y, int small_or_large);
	// Utility Functions
		void invert_channel_order(int channel);
		int _size_comp(const void *, const void *);
		int _x_comp(const void *, const void *);
		int _y_comp(const void *, const void *);
		int _dist_comp(const void *, const void *);
		unsigned int fastroot(unsigned int x);

//Definitions

/// ///// ///
/// SETUP ///
/// ///// ///

/*	Initialization function.  This should be called in main() before
	any of the hhs_vision_lib functions are utilized.
*/
void init_hhs_vision(){
	if(!_hhs_vision_initted){
		clear_all_blob_data();
		#ifdef FILEPRINT
		system("mkdir -p /mnt/user/tmpusb");
		system("mount /dev/sdb1 /mnt/user/tmpusb -t vfat");
		#endif
		printf("HHS Vision Library\nAdam Farabaugh and Evan Wilson\nVISION SYSTEM INITIALIZED");
		_hhs_vision_initted = 1;
	}
}

void close_hhs_vision(){
	if(_hhs_vision_initted){
		#ifdef FILEPRINT
		system("unmount /dev/sdb1");
		#endif
		printf("HHS Vision Library\nVISION SYSTEM CLOSED\n");
		_hhs_vision_initted = 0;
	}
}

/// //////////// ///
/// GETTING DATA ///
/// //////////// ///

/*	Stores data about a blob into a structure which we can work with
	more easily than KIPR's proprietary vision data storage (this library would
	be infinitely better if we could access the KIPR data directly but
	I have not had time to search through the firmware to find the arrays
	where it is stored by the KIPR tracking lib)
*/
BLOB get_blob_data(int channel, int blob_index){
	init_hhs_vision();
	BLOB blob;
	blob.size = track_size(channel, blob_index);
	blob.x = track_x(channel, blob_index);
	blob.y = track_y(channel, blob_index);
	blob.bbl = track_bbox_left(channel, blob_index);
	blob.bbr = track_bbox_right(channel, blob_index);
	blob.bbt = track_bbox_top(channel, blob_index);
	blob.bbb = track_bbox_bottom(channel, blob_index);	
	return(blob);
}

/*	Gets all data about blobs in one channel in one step
*/
void get_channel_blob_data(int channel){
	init_hhs_vision();
	int i;
	for(i=0;i<NUM_BLBS;i++){
		vision_data_array[channel][i] = get_blob_data(channel,i);
	}
}

/*	Gets all data about blobs in all four channels in one step
*/
void get_all_blob_data(){
	init_hhs_vision();
	int i;
	int j;
	for(i=0;i<4;i++){
		for(j=0;j<NUM_BLBS;j++){
			vision_data_array[i][j] = get_blob_data(i,j);
		}	
	}
}

/// /////////////// ///
/// DATA MANAGEMENT ///
/// /////////////// ///

/*	Clears the blob data for one index of the vision_data_array
*/
void clear_blob_data(int channel, int i){
	init_hhs_vision();
	vision_data_array[channel][i].size = -1;
	vision_data_array[channel][i].x = -1;
	vision_data_array[channel][i].y = -1;
	vision_data_array[channel][i].bbl = -1;
	vision_data_array[channel][i].bbr = -1;
	vision_data_array[channel][i].bbt = -1;
	vision_data_array[channel][i].bbb = -1;
	vision_data_array[channel][i].dist = -1.0;
}

/*	Clears the entire vision_data_array of all blob data
*/
void clear_all_blob_data(){
	init_hhs_vision();
	int i, j;
	for(i=0;i<NUM_CHS;i++){
		for(j=0;j<NUM_BLBS;j++){
			clear_blob_data(i,j);
		}	
	}
}

/*	Cleans up a channel of vision_data_array by condensing all
	clear indices at the end (probably mostly useful for printing
	data or logging it to file
*/
void clean_vision_array(int channel){
	init_hhs_vision();
	int i, j;
	for(i=0;i<NUM_BLBS;i++){
		if(vision_data_array[channel][i].size == -1){
			for(j=i;j<9;j++){
				vision_data_array[channel][j] = vision_data_array[channel][j+1];
			}
			clear_blob_data(channel,NUM_BLBS);
		}
	}
}

void print_vision_array(){
	int i;
	for(i=0;i<NUM_CHS;i++){
		print_one_channel(i);
	}
}

void print_one_channel(int ch){
	init_hhs_vision();
	int i, j;
	
	//time_t ltime; /* calendar time */
    //ltime=time(NULL); /* get current cal time */
	#ifdef FILEPRINT
		FILE *debug;
		debug=fopen("/mnt/user/tmpusb/debug.txt", "a+");
		//fprintf(debug, "\n\n%s\n", asctime(localtime(&ltime)));
		fprintf(debug, "| c | i | size |  x  |  y  | d | bbl | bbr | bbt | bbb |\r\n");
	#endif
	#ifdef SCREENPRINT
		//printf("\n\n%s\n", asctime(localtime(&ltime)));
		printf("| c | i | size |  x  |  y  | d |\n");
	#endif
	for(i=0;i<NUM_BLBS;i++){
		BLOB b=vision_data_array[j][i];
		if(b.size != -1){
			#ifdef SCREENPRINT
				printf("| %d | %d | %04d | %03d | %03d | %03d |\n",j,i,b.size,b.x,b.y,b.dist);
			#endif
			#ifdef FILEPRINT
				fprintf(debug,"| %d | %d | %04d | %03d | %03d | %03d | %03d | %03d | %03d | %03d |\r\n",j,i,b.size,b.x,b.y,b.dist,b.bbl,b.bbr,b.bbt,b.bbb);
			#endif
		}
	}
	#ifdef FILEPRINT
	fclose(debug);
	#endif
}

/// ////////////// ///
/// BLOB FILTERING ///
/// ////////////// ///

/*	Filters out all blobs with size less or greater than threshold.
	If less_or_greater==1, filters out those that are smaller; if
	less_or_greater==-1, filters out those that are larger.  
*/
void filter_by_size(int channel, int threshold, int less_or_greater){
	int i;
	for(i=0;i<NUM_BLBS;i++){
		if(vision_data_array[channel][i].size != -1){
			if((vision_data_array[channel][i].size < threshold) * less_or_greater){
				clear_blob_data(channel,i);
			}
		}
	}
}

/*	Filters out all blobs with x-coord less or greater than threshold.
	If less_or_greater==1, clears those that are to the left; if
	less_or_greater==-1, clears those that are to the right.  
*/
void filter_by_x(int channel, int threshold, int less_or_greater){
	int i;
	for(i=0;i<NUM_BLBS;i++){
		if(vision_data_array[channel][i].size != -1){
			if((vision_data_array[channel][i].x < threshold) * less_or_greater){
				clear_blob_data(channel,i);
			}
		}
	}
}

/*	Filters out all blobs with x-coord less or greater than threshold.
	If less_or_greater==1, clears those that are above; if
	less_or_greater==-1, clears those that are below.  
*/
void filter_by_y(int channel, int threshold, int less_or_greater){
	int i;
	for(i=0;i<NUM_BLBS;i++){
		if(vision_data_array[channel][i].size != -1){
			if((vision_data_array[channel][i].y < threshold) * less_or_greater){
				clear_blob_data(channel,i);
			}
		}
	}
}

/*	Filters out all blobs either in or not in the box defined by
	the upper left (x1,y1) and lower right (x2,y2) points.  If 
	internal_or_external==1, keeps only blobs inside box.  If 
	internal_or_external==-1, keeps only blobs outside of box.
*/
void filter_by_box(int channel, int x1, int y1, int x2, int y2, int internal_or_external){
	filter_by_x(channel, x1, 1*internal_or_external);
	filter_by_x(channel, x2, -1*internal_or_external);
	filter_by_y(channel, y1, 1*internal_or_external);
	filter_by_y(channel, y2, -1*internal_or_external);
}

/// //////////// ///
/// BLOB MERGING ///
/// //////////// ///

/*	Merges all attributes of two blobs from the same
	channel.
*/
void merge_blobs(int channel, int i1, int i2){
	BLOB temp_blob;
	
	if(vision_data_array[channel][i1].size != -1 && vision_data_array[channel][i2].size != -1){
		temp_blob.size = vision_data_array[channel][i1].size + vision_data_array[channel][i2].size;
		temp_blob.bbl = fmin(vision_data_array[channel][i1].bbl, vision_data_array[channel][i2].bbl);
		temp_blob.bbr = fmax(vision_data_array[channel][i1].bbr, vision_data_array[channel][i2].bbr);
		temp_blob.bbt = fmin(vision_data_array[channel][i1].bbt, vision_data_array[channel][i2].bbt);
		temp_blob.bbb = fmax(vision_data_array[channel][i1].bbb, vision_data_array[channel][i2].bbb);
		temp_blob.x = (int)((temp_blob.bbr + temp_blob.bbl) / 2.);
		temp_blob.y = (int)((temp_blob.bbt + temp_blob.bbb) / 2.);
		
		vision_data_array[channel][i1] = temp_blob;
		clear_blob_data(channel, i2);
	}
	else{
		printf("WARN: merge_blobs failed: one or both blobs are clear of data\n");
	}
}

/*	Merges blobs on the same channel if their distance
	is less than threshhold.
*/
void merge_by_distance(int channel, int threshold){
	int i, j;
	int xd, yd;
	int d;
	for(i=0;i<NUM_BLBS;i++){
		if(vision_data_array[channel][i].size != -1){
			for(j=0;j<NUM_BLBS;j++){
				if(vision_data_array[channel][j].size != -1){
					xd = vision_data_array[channel][i].x - vision_data_array[channel][j].x;
					yd = vision_data_array[channel][i].y - vision_data_array[channel][j].y;
					d = fastroot(xd*xd+yd*yd);
					if(d < threshold)
						merge_blobs(channel, i, j);
				}
			}
		}
	}
}

/*	Merges blobs on the same channel if they are overlapping
	or their bounds intersect (check if TL or BR corner is inside
	another blob).
*/
void merge_by_bounds(int channel, int threshhold){
	int i, j;
	for(i=0;i<NUM_BLBS;i++){
		if(vision_data_array[channel][i].size != -1){
			for(j=0;j<NUM_BLBS;j++){
				if(vision_data_array[channel][j].size != -1){
					if(((vda[channel][i].bbt > vda[channel][j].bbt) &&
						(vda[channel][i].bbl > vda[channel][j].bbl) &&
						(vda[channel][i].bbt < vda[channel][j].bbb) &&
						(vda[channel][i].bbl < vda[channel][j].bbr))
						||
					   ((vda[channel][i].bbb < vda[channel][j].bbb) &&
						(vda[channel][i].bbr < vda[channel][j].bbr) &&
						(vda[channel][i].bbb > vda[channel][j].bbt) &&
						(vda[channel][i].bbr > vda[channel][j].bbl))){
						merge_blobs(channel, i, j);
					}
				}
			}
		}
	}
}

/*	Merges channels c1 and c2 into channel ctarget (this is what the 5th
	channel is for).  For now, the only attribute this function will filter
	by, if there are more than NUM_BLBS total blobs to merge into one 
	channel, is size.
*/
void merge_channels(int c1, int c2, int ctarget, int order){
	int t = 0;
	int n1, n2;
	int i, j, k;
	// determine size of arrays to merge
	for(i=0;i<NUM_BLBS;i++){
		if(vision_data_array[c1][i].size != -1){n1++;}
	}
	for(i=0;i<NUM_BLBS;i++){
		if(vision_data_array[c2][i].size != -1){n2++;}
	}
	// decide if they will all fit or not
	if(n1 + n2 <= NUM_BLBS){ // they'll fit
		k=0;		
		for(i=0;i<NUM_BLBS;i++){
			if(vision_data_array[c1][i].size != -1){
				vision_data_array[ctarget][k] = vision_data_array[c1][i];
				k++;
			}
		}
		k=0;
		for(i=0;i<NUM_BLBS;i++){
			if(vision_data_array[c2][i].size != -1){
				vision_data_array[ctarget][k] = vision_data_array[c2][i];
				k++;
			}
		}
	}
	else{ // they won't fit so we have to get creative
		k=0;
		n1=0;
		n2=0;
		sort_by_size(c1, order);
		sort_by_size(c2, order);
		while(k<NUM_BLBS){
			t = (vision_data_array[c1][n1].size >= vision_data_array[c2][n2].size);
			if(order != 1){t = !t;}
			if(t){
				vision_data_array[ctarget][k] = vision_data_array[c1][n1];
				k++;
				n1++;
			}		
			else{
				vision_data_array[ctarget][k] = vision_data_array[c2][n2];
				k++;
				n2++;
			}			
		}
	}
}

/// //////////// ///
/// BLOB SORTING ///
/// //////////// ///

/*	These following functions use the stdlib.h qsort() function
	to sort the vision data array by various elements.  If the passed
	var small_or_large==1 the order is left as qsort returns it.  If
	small_or_large==-1 the order is inverted after qsort returns.
*/
void sort_by_size(int channel, int small_or_large){
	qsort((void *) &vision_data_array[channel], NUM_BLBS, sizeof(BLOB), _size_comp);
	if(small_or_large==-1){invert_channel_order(channel);}
}

void sort_by_x(int channel, int small_or_large){
	qsort((void *) &vision_data_array[channel], NUM_BLBS, sizeof(BLOB), _x_comp);
	if(small_or_large==-1){invert_channel_order(channel);}
}

void sort_by_y(int channel, int small_or_large){
	qsort((void *) &vision_data_array[channel], NUM_BLBS, sizeof(BLOB), _y_comp);
	if(small_or_large==-1){invert_channel_order(channel);}
}

/*	This first calculates each blob's distance from a specified point
	and then sorts vision_data_array by this distance, according to
	the above specifications.
*/
void sort_by_point(int channel, int point_x, int point_y, int small_or_large){
	int i;
	int xd, yd;
	int d;
	for(i=0;i<NUM_BLBS;i++){
		if(vision_data_array[channel][i].size != -1){
			xd = vision_data_array[channel][i].x - point_x;
			yd = vision_data_array[channel][i].y - point_y;
			d = fastroot(xd*xd+yd*yd);
			vision_data_array[channel][i].dist = d;
		}
	}
	qsort((void *) &vision_data_array[channel], NUM_BLBS, sizeof(BLOB), _dist_comp);
	if(small_or_large==-1){invert_channel_order(channel);}
}

/// ///////////////// ///
/// UTILITY FUNCTIONS ///
/// ///////////////// ///

/*	Flips the order of blobs in a channel (i.e. if sorted big to small,
	this function will make the channel array small to big).
*/
void invert_channel_order(int channel){
	BLOB temp_blob;
	int i=1, j=NUM_BLBS;
	while(i < j){
		temp_blob = vision_data_array[channel][i];
		vision_data_array[channel][i] = vision_data_array[channel][j];
		vision_data_array[channel][j] = temp_blob;
		i++;
		j--;
	}
}

/*	The following several functions serve as utility functions
	to pass to qsort() to compare various elements of structures
	of type BLOB
*/
int _size_comp(const void *e1, const void *e2){
	const BLOB *b1=e1;
	const BLOB *b2=e2;
	
	if(b1->size < b2->size) return -1;
	else if(b1->size > b2->size) return 1;
	else return 0;
}

int _x_comp(const void *e1, const void *e2){
	const BLOB *b1=e1;
	const BLOB *b2=e2;
	
	if(b1->x < b2->x) return -1;
	else if(b1->x > b2->x) return 1;
	else return 0;
}

int _y_comp(const void *e1, const void *e2){
	const BLOB *b1=e1;
	const BLOB *b2=e2;
	
	if(b1->y < b2->y) return -1;
	else if(b1->y > b2->y) return 1;
	else return 0;
}

int _dist_comp(const void *e1, const void *e2){
	const BLOB *b1=e1;
	const BLOB *b2=e2;
	
	if(b1->dist < b2->dist) return -1;
	else if(b1->dist > b2->dist) return 1;
	else return 0;
}

/*	Approximates the first root of x using 3 iterations
	of Newton's approximation method.  Over the unsigned
	integer space, it deviates by no more than 20.
*/
unsigned int fastroot(unsigned int x){
	unsigned int a, b;
	b=x;
	a=x=0x3f;
	x=b/x;
	a=x=(x+a)>>1;
	x=b/x;
	a=x=(x+a)>>1;
	x=b/x;
	x=(x+a)>>1;
	return(x);
}

