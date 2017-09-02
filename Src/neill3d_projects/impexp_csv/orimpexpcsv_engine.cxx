
/**	\file	orimpexpcsv_engine.cxx
*/


/////////////////////////////////////////////////////////////////////////////////////////
//
// Licensed under the "New" BSD License. 
//		License page - https://github.com/Neill3d/MoBu/blob/master/LICENSE
//
// GitHub repository - https://github.com/Neill3d/MoBu
//
// Author Sergey Solohin (Neill3d) 2014
//  e-mail to: s@neill3d.com
//		www.neill3d.com
/////////////////////////////////////////////////////////////////////////////////////////

// todo: some data is rize up
// ImportFrameData(buffer);

#include "orimpexpcsv_engine.h"
#include <math.h>
#include <stdio.h>

#include "TextUtil.h"

/************************************************
 * File importation function
 * - to be modified to import your own custom file
 * - format. For examples, see the scene creation
 * - functions.
 ************************************************/
FBModelOptical *ORImportCsvEngine::CreateOpticalModel(int frameCount)
{
	
	FBTime	start = 0;
	FBTime	stop;
	FBTime	oneFrame(0,0,0,1,0,kFBTimeMode100Frames);

	// set stop time
	FBPlayerControl().SetTransportFps( kFBTimeMode100Frames );
	stop.SetMilliSeconds(frameCount*oneFrame.GetMilliSeconds());
	FBPlayerControl().LoopStop = stop;

	FBModelOptical  *model = new FBModelOptical( "CSV" );

	// set sampling characteristics
	model->SamplingStart = start;
	model->SamplingStop = stop;
	model->SamplingPeriod = oneFrame;

	

	char name[256];
	int sampleCount, ndx;
	double lPos[3];
	FBModelMarkerOptical  *opticalMarker;
	FBModelList		lList;
/*
	// for number of rigidbodies
	for (int i=0; i<csv.rigidbodycount-1; i++){

		lList.Clear();
		csv.rigidbodies[i]->print_marker_setup();
		
		for (int j=0; j<csv.rigidbodies[i]->markercount; j++){
			sprintf(name, "rg%d%d", i,j);
			opticalMarker = new FBModelMarkerOptical(name);
			opticalMarker->Show = true;
			model->Children.Add(opticalMarker);

			lList.Add(opticalMarker);

			memcpy(lPos, csv.rigidbodies[i]->marker_setup[j].pos, sizeof(double)*3);

			model->ImportSetup();

			opticalMarker->ImportBegin();
			opticalMarker->ImportKey(	lPos[0],
										lPos[1],
										lPos[2] );
			opticalMarker->ImportEnd();

			//opticalMarker->SetVector( csv.rigidbodies[i].marker_setup[j].pos );
			printf("pos[%d, %d] - %f; %f; %f\n", i, j, lPos[0], lPos[1], lPos[2] );
		}


		model->RigidBodies.Add( lList, csv.rigidbodies[i]->name );
		model->RigidBodies[model->RigidBodies.GetCount() - 1].Snap();
	}
*/
	// for number of markers
	

	for (int i=0; i<csv.markercount; i++)
	{
		sprintf(name, "m%d", i);
		opticalMarker = new FBModelMarkerOptical(name);
		opticalMarker->Show = true;
		model->Children.Add(opticalMarker);
	}

	// start, stop, period
	model->ImportSetup();

	for (int i=0; i<csv.markercount; i++)
	{
		ndx=0;
		memset(lPos, 0, sizeof(double)*3);

		opticalMarker = (FBModelMarkerOptical*) model->Children[i];

		sampleCount = opticalMarker->ImportBegin();
		for(int j=0;j<sampleCount; j++)
		{
			if (j < csv.markers[i].firstframe) {
				// skip
				opticalMarker->ImportKey(lPos[0],lPos[1],lPos[2],1.0);
			} else {
				memcpy(lPos, csv.markers[i].data[ndx].pos, sizeof(double)*3);
				opticalMarker->ImportKey(lPos[0],lPos[1],lPos[2]);

				ndx++;
				if (ndx >= csv.markers[i].data.GetCount() ) break;
				csv.markers[i].firstframe = csv.markers[i].data[ndx].frame;
			}
		}
		opticalMarker->ImportEnd();
	}
	
	
/*
	for (int i=0; i<csv.rigidbodycount; i++)
	{
		opticalMarker = new FBModelMarkerOptical(csv.rigidbodies[i]->name);
		opticalMarker->Show = true;
		opticalMarker->Done = false;
		model->Children.Add(opticalMarker);
	}

	model->ImportSetup();
	for (int i=0; i<csv.rigidbodycount; i++)
	{
		opticalMarker = (HFBModelMarkerOptical)model->Children[i];
		ndx=0;
		memset(lPos, 0, sizeof(double)*3);
		sampleCount = opticalMarker->ImportBegin();
		for (int j=0; j<sampleCount; j++)
		{
			if (j < csv.rigidbodies[i]->firstframe) {
				// skip
				opticalMarker->ImportKey(lPos[0],lPos[1],lPos[2]);
			} else {
				memcpy(lPos, csv.rigidbodies[i]->data[ndx].pos, sizeof(double)*3);
				opticalMarker->ImportKey(lPos[0],lPos[1],lPos[2]);

				ndx++;
				if (ndx >= csv.rigidbodies[i]->data.GetCount() ) break;
				csv.rigidbodies[i]->firstframe = csv.rigidbodies[i]->data[ndx].frame;
			}
		}
		opticalMarker->ImportEnd();
	}
*/
	/*
	// filtered markers
	for (int i=0; i<csv.markers_filtered.GetCount(); i++)
	{
		sprintf(name, "m%d", i);
		opticalMarker = new FBModelMarkerOptical(name, model);
		opticalMarker->Show = true;
		model->Children.Add(opticalMarker);
	}

	// start, stop, period
	model->ImportSetup();

	for (i=0; i<csv.markers_filtered.GetCount(); i++)
	{
		ndx=0;
		memset(lPos, 0, sizeof(double)*3);

		opticalMarker = (HFBModelMarkerOptical) model->Children[i];

		sampleCount = opticalMarker->ImportBegin();
		for(int j=0;j<sampleCount; j++)
		{
			if (j < csv.markers_filtered[i].firstframe) {
				// skip
				opticalMarker->ImportKey(lPos[0],lPos[1],lPos[2],1.0);
			} else {
				memcpy(lPos, csv.markers_filtered[i].data[ndx].pos, sizeof(double)*3);
				opticalMarker->ImportKey(lPos[0],lPos[1],lPos[2], csv.markers_filtered[i].data[ndx].occulusion);

				ndx++;
				if (ndx >= csv.markers_filtered[i].data.GetCount() ) break;
				csv.markers[i].firstframe = csv.markers_filtered[i].data[ndx].frame;
			}
		}
		opticalMarker->ImportEnd();
	}
*/
	// add to scene
	model->Show = true;


	return model;
}

void ORImportCsvEngine::CreateOpticalRigidBody( char* line )
{
	FBVector3d  lPos;

	int count = 0;
	int n=0;
	char *p;
	char name[80];
	char markerName[80];

	memset(name, 0, sizeof(char)*80);
	memset(markerName, 0, sizeof(char)*80);

	csv_data::rigidbody		*rigidbody = new csv_data::rigidbody;

	p = strtok( line, "," );
	printf( "%s\n", p );
	do {
		p = strtok(NULL, "," );
		n++;

		if(p) {

			switch(n) {
			case 1:
				// name of the rigid body
				strncpy(name, p+1, strlen(p)-2);
				printf( "ribig body name |%s|\n", name );
				break;
			case 2:
				// rigidbody ID
				printf("scan rigidbody id\n" );
				sscanf(p, "%d", &count);
				printf( "rigidbody  ID %d\n", count );
				//rigidbody=csv.rigidbodies[count-1];
				strcpy(rigidbody->name, name);

				printf( "rigidbody  ID %d\n", count );
				break;
			case 3:
				// count of the markers in the rigid body
				sscanf(p, "%d", &rigidbody->markercount);
				rigidbody->marker_setup.SetCount(rigidbody->markercount);
				break;
			default:
				if (n>3) {
					int markerNdx;
					markerNdx = (n-4);
					markerNdx = markerNdx%3;
					float value;
					sscanf(p, "%f", &value);

					lPos[markerNdx] = value*100;

					if (markerNdx==2) {
						markerNdx = (n-4) / 3;
						memcpy(rigidbody->marker_setup[markerNdx].pos, lPos, sizeof(double)*3);

						printf("marker[%d].pos - %f; %f; %f\n", markerNdx, lPos[0], lPos[1], lPos[2] );
						rigidbody->marker_setup[markerNdx].print();
					}
				}
				break;
			}
			printf( "%s\n", p );
		}

	} while(p);

	csv.rigidbodies[count-1]=rigidbody;
	csv.rigidbodies[0]->print_marker_setup();

/*
	model->RigidBodies.Add( lList, name );
	model->RigidBodies[model->RigidBodies.GetCount() - 1].Snap();

	FBRigidBody rigidbody = model->RigidBodies[model->RigidBodies.GetCount()-1];

	printf("list count - %d\n", rigidbody.Markers.GetCount() );
	for(int i=0; i<rigidbody.Markers.GetCount(); i++)
	{
		HFBModel marker = rigidbody.Markers[i];
		marker->GetVector( lPos, kModelTranslation, true );

		printf("pos - %f; %f; %f\n", lPos[0], lPos[1], lPos[2] );
	}
*/
	//printf( "|%s| - |%s|\n", temp2, temp );
}

void ORImportCsvEngine::ImportFrameData( char *line )
{
	FBVector3d	lPos;
	FBVector4d	lQuat;
	FBVector3d	lRot;  // euler angles

	int frame=0;
	int rigidbodies=0;
	int markers=0;
	int n=0;

	char *p;
	p = strtok( line, "," );
	printf( "%s\n", p );
	do {
		p = strtok(NULL, "," );
		n++;

		if(p) {

			switch(n) {
			// frame ID
			case 1:
				sscanf(p, "%d", &frame);
				printf( "local frame |%d|\n", frame );

				break;
			case 2:
				// number of rigid bodies tracked in current frame
				sscanf(p, "%d", &rigidbodies);
				printf( "number of frame rigid bodies |%d|\n", rigidbodies );

				// rigid bodies information
				int i,j;
				for(i=0; i<rigidbodies; i++){
					int rigidbodyNdx = 0;
					// rigidbody ID
					p = strtok(NULL, ",");
					n++;
					if (!p) break;
					sscanf(p, "%d", &rigidbodyNdx);
					printf("rigidbody index %d\n", rigidbodyNdx);
					// pos
					for (j=0; j<3; j++) {
						p = strtok(NULL, ",");
						n++;
						if (!p) break;

						float value;
						sscanf(p, "%f", &value);
						lPos[j] = value*100;
					}
					// quat
					for (j=0; j<4; j++) {
						p = strtok(NULL, ",");
						n++;
						if (!p) break;

						float value;
						sscanf(p, "%f", &value);
						lQuat[j] = value;
					}
					// euler angles (heading, attitude, bank)
					/*
					for (j=0; j<3; j++) {
						p = strtok(NULL, ",");
						n++;
						if (!p) break;

						float value;
						sscanf(p, "%f", &value);
						lRot[j] = value;
					}
					*/

					csv_data::rigidbody_data	rigidbody_data(frame, lPos, lQuat, lRot);
					csv.rigidbodies[rigidbodyNdx-1]->data.Add(rigidbody_data);
				}

				p = strtok(NULL, ",");
				n++;
				if (!p) break;

				sscanf(p, "%d", &markers);
				printf( "number of frame markers |%d|\n", markers );

				if (markers > csv.markercount) {

					for(int i=csv.markercount; i<markers; i++){
						csv_data::marker		marker(frame);
						csv.markers.Add(marker);
					}

					csv.markercount = markers;
				}

				break;
			default:
				if (markers>0) {
					int i;
					for (i=0; i<markers; i++)
					{
						for (int j=0; j<3; j++) {
							p = strtok(NULL, ",");
							n++;
							if (!p) break;

							float value;
							sscanf(p, "%f", &value);
							lPos[j] = value*100;
						}

						csv_data::marker_data	framedata(frame, lPos);

						//printf( "import marker key - [%f, %f, %f]\n", lPos[0], lPos[1], lPos[2] );
						csv.markers[i].data.Add(framedata);
					}
				}
				break;
			}
			//printf( "%s\n", p );
		}

	} while(p);
	
}

void ORImportCsvEngine::FilterFrameData()
{
	FBVector3d pos;	
	bool inserted=false;

	{for(int frame=0; frame<csv.framecount; frame++)
	{

		for (int i=0; i<csv.markercount; i++)
		{
			if (csv.markers[i].firstframe > frame) continue;
			if (!csv.markers[i].GetFramePos(frame, &pos) ) continue;

			inserted = false;
			for (int j=0; j<csv.markers_filtered.GetCount(); j++)
			{
				inserted = csv.markers_filtered[j].test(frame, pos);
				if (inserted) break;
			}

			if (!inserted) {
				csv_data::marker_filtered	marker_filtered;
				marker_filtered.insert(frame, pos);
				csv.markers_filtered.Add(marker_filtered);
				
			}
		}

		for (int i=0; i<csv.markers_filtered.GetCount(); i++)
			csv.markers_filtered[i].insertOcculuded(frame);
	}}
}

bool ORImportCsvEngine::ImportFile( const char* pFileName )
{
	printf("importing file... %s\n", pFileName );

	// if successful importation, return true
	FILE *f;

	f = fopen( pFileName, "r" );
	if ( f == NULL ) return false;

	char buffer[2048];

	{while(!feof(f) ) {
		memset(buffer,0,sizeof(char)*2048);
		fgets(buffer, 2048, f);

		if (strstr(buffer, "info") == buffer) {
			// whole optical model info

			char* ptr=NULL;
			ptr = strstr(buffer, "framecount");
			if (ptr) {
				// read framecount
				GetTextPart(buffer,2, ptr);
				if (ptr) {
					sscanf( ptr, "%d", &csv.framecount );

					//: create optical model
					/*
					modelRoot = CreateOpticalModel(frameCount);
					if (modelRoot->ImportSetup() == false ) {
						printf("optical model is not ready...\n");
					}
					*/

					delete [] ptr;
					ptr=NULL;
				}
			}
			else {
				ptr = strstr(buffer, "rigidbodycount");
				if (ptr) {
					//read rigidbodycount
					GetTextPart(buffer,2, ptr);
					if (ptr) {
						sscanf( ptr, "%d", &csv.rigidbodycount );
						csv.rigidbodies.SetCount(csv.rigidbodycount);
						printf("rigidbodies count %d", csv.rigidbodycount);
						delete [] ptr;
						ptr=NULL;
					}
				}
			}

		}
		else
		if (strstr(buffer, "rigidbody") == buffer ) {
			//rigid body stats
			CreateOpticalRigidBody(buffer);
		}
		else
		if (strstr(buffer, "frame") == buffer ) {
			//reading each frame data
			ImportFrameData(buffer);
			//FilterFrameData();
		}
	}}

	fclose(f);

	/*
	// end parsing markers data
	printf("markers count %d\n", modelRoot->Markers.GetCount() );
	for (int i=0; i<modelRoot->Markers.GetCount(); i++)
		modelRoot->Markers[i]->ImportEnd();
		*/
	
	CreateOpticalModel(csv.framecount);
	csv.FreeData();

	return true;
}



/************************************************
 *	Find an animation node from a given root.
 ************************************************/
FBAnimationNode *ORImportCsvEngine::FindAnimationNode( FBAnimationNode *pNode, char* pName )
{
	// For the number of child nodes on pNode.
	for(int i=0;i < pNode->Nodes.GetCount();i++)
	{
		FBString str(pNode->Nodes[i]->Name);
		const char *lName = str;

		// If the same, return the node.
		if(strcmp(lName,pName)==0)
		{
			return pNode->Nodes[i];
		}
	}

	return NULL;
}



