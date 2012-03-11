/*	hhs_vision_lib.c
	Hampton High School Botball Team
	Team #0141
	DC Region
	
	Adam Farabaugh - adam.farabaugh1@gmail.com
	Evan Wilson - emwilson02@gmail.com
	
	Change Log:
		Apr 23 2011		- created
*/

#include "hhs_vision_lib.c"

int main(){
	init_hhs_vision();
	clear_all_blob_data();
	track_update();
	get_all_blob_data();
	
	// merges channels 0 and 2, putting result in virtual channel 5
	// prefers large blobs over small ones if there are more than 10 total
	merge_channels(0, 2, 5, LARGE);
	// sorts the virtual channel (with merged info from 0 and 2) around the
	// center of the screen, preferring smaller distances to the center of the image
	sort_by_point(5, 80, 60, SMALL);
	
	printf("Block coords: x=%d, y=%d", vision_data_array[5][0].x, vision_data_array[5][0].y);

	close_hhs_vision();
}

