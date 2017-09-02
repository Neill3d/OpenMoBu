

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

/**	\file	orimpexpcsv_engine.cxx
*/

// todo: some data is rize up
// ImportFrameData(buffer);

#include "orimpexpcsm_engine.h"
#include <math.h>
#include <stdio.h>

#include "TextUtil.h"

#include <array>

/************************************************
 * File importation function
 * - to be modified to import your own custom file
 * - format. For examples, see the scene creation
 * - functions.

 // TODO
 1. dropout import support
 + 2. config ini file

 ************************************************/
FBModelOptical *CSMEngine::CreateOpticalModel(int frameCount)
{
	FBConfigFile	lConfig("@CSM.txt");

	FBVector3d	pos;
	FBTime	start;
	FBTime	stop;
	
	// this is for mobu 2010
	//FBTime	oneFrame(0,0,0,1,0,kFBTimeModeCustom, (double)csm.framerate);
	
	// this is a new FBTime constructor
	FBTime	oneFrame(0,0,0,1,0,kFBTimeModeCustom);
	oneFrame.SetSecondDouble( 1.0 / csm.framerate );

	// set stop time
	FBPlayerControl().SetTransportFps( kFBTimeModeCustom, (double)csm.framerate );
	start.SetMilliSeconds( csm.firstframe * oneFrame.GetMilliSeconds() );
	stop.SetMilliSeconds( csm.lastframe * oneFrame.GetMilliSeconds() );
	//FBPlayerControl().LoopStart = start;
	//FBPlayerControl().LoopStop = stop;

	FBModelOptical  *model = new FBModelOptical( "CSM" );

	// set sampling characteristics
	model->SamplingStart = start;
	model->SamplingStop = stop;
	model->SamplingPeriod = oneFrame;
	
	int sampleCount;
	FBModelMarkerOptical  *opticalMarker;

	// for number of markers
	for (int i=0; i<csm.markers.GetCount(); i++)
	{
		csm_data::marker_name *ptr = csm.markers.GetAt(i);
		
		opticalMarker = new FBModelMarkerOptical( (ptr) ? ptr->name : "opticalMarker" );
		opticalMarker->Show = true;
		opticalMarker->Size = 150.0;
		opticalMarker->SetModelOptical(model);
		//model->Children.Add(opticalMarker);
	}

	const int bufferSize = 256;
	char buffer[bufferSize];
	
	memset( buffer, 0, sizeof(char) * bufferSize );
	sprintf_s(buffer, bufferSize, "0");

	const char *bufferPtr = buffer;
	lConfig.GetOrSet( "IMPORT", "rightCoordSys", bufferPtr );
	char rCoordSys = bufferPtr[0];

	sprintf_s(buffer, bufferSize, "0.0925");
	bufferPtr = buffer;
	lConfig.GetOrSet( "IMPORT", "scale", bufferPtr );
	
	float scale;
	sscanf( bufferPtr, "%f", &scale );

	// start, stop, period
	model->ImportSetup();
	
	for (int i=0; i<csm.markers.GetCount(); i++)
	{
		if (i >= model->Markers.GetCount())
			continue;

		opticalMarker = (FBModelMarkerOptical*) model->Markers[i];

		sampleCount = opticalMarker->ImportBegin();
		for (int j=0; j<csm.points.GetCount(); j++)
		{
			if (j >= sampleCount)
				break;

			// left handed layout
			if (rCoordSys == '1')
			{	
				pos[0] = (double)csm.points[j].pos[i*3] * scale;
				pos[1] = (double)csm.points[j].pos[i*3+2] * scale;
				pos[2] = (double)-csm.points[j].pos[i*3+1] * scale;
			}
			else
			{
				pos[0] = (double)csm.points[j].pos[i*3] * scale;
				pos[1] = (double)csm.points[j].pos[i*3+1] * scale;
				pos[2] = (double)csm.points[j].pos[i*3+2] * scale;
			}

			bool result = false;
			if ( (pos[0] == 0.0) && (pos[1] == 0.0) && (pos[2] == 0.0))
				result = opticalMarker->ImportKey(0.0, 0.0, 0.0, 1.0);
			else
				result = opticalMarker->ImportKey( pos[0], pos[1], pos[2] );

			if (result == false)
				break;
		}
		bool result = opticalMarker->ImportEnd();
		if (result == false)
			break;
	}
	

	// add to scene
	model->Show = true;
	return model;
}

void CSMEngine::CreateOpticalRigidBody( char* line )
{

}

void CSMEngine::ImportFrameData( char *line )
{
	
}

bool CSMEngine::ExportFile( const char* pFileName )
{
	FBConfigFile	lConfig("@CSM.txt");

	FBProgress	lProgress;
	lProgress.Caption = "ExportCSM";
	lProgress.Text = "exporting file...";
	lProgress.Percent = 0;

	printf("exporting file... %s\n", pFileName );

	FILE *f;
	f = fopen( pFileName, "w" );
	if ( f == NULL ) return false;
	fseek(f, 0, 0);
#ifdef OLD_FIND_MODEL_BY_NAME
	FBModelOptical *pModel = (FBModelOptical*) FBFindModelByName("CSM");
#else
	FBModelOptical *pModel = (FBModelOptical*) FBFindModelByLabelName("CSM");
#endif
	if (!pModel) return false;

	// info
	fprintf(f, "$Filename test.csm\n" );
	fprintf(f, "$Data 2009/02/06\n" );
	fprintf(f, "$Time 18:33:00\n" );
	fprintf(f, "$Actor test 1\n" );
	fprintf(f, "\n" );

	fprintf(f, "$Comments\n" );
	fprintf(f, "General Test\n" );
	fprintf(f, "\n" );

	// optical model statistics
	FBTime		lTime;
	int		startFrame, stopFrame;

	lTime = pModel->SamplingStart;
	startFrame = lTime.GetFrame();
	fprintf(f, "$FirstFrame %d\n", startFrame );
	lTime = pModel->SamplingStop;
	stopFrame = lTime.GetFrame();
	fprintf(f, "$LastFrame %d\n", stopFrame );
	lTime = pModel->SamplingPeriod;
	//double rate = lTime.GetSecondDouble() * 10000.0;

	std::array<char, 256> buffer;

	buffer.fill(0);
	sprintf_s(buffer.data(), buffer.size(), "120.0" );

	const char *cbufferPtr = buffer.data();
	
	float rate;
	lConfig.GetOrSet( "EXPORT", "Rate", cbufferPtr );
	
	sscanf( cbufferPtr, "%f", &rate );

	fprintf(f, "$Rate %f\n", rate );				// !! >> !!
	fprintf(f, "\n" );

	// output markers order
	fprintf( f, "$Order\n" );
	for (int i=0; i<pModel->Children.GetCount(); i++)
		if (pModel->Children[i]->Is(FBModelMarkerOptical::TypeInfo))
		{
			FBString str( pModel->Children[i]->Name );
			fprintf( f, "%s ", (char*) str );
		}

	fprintf(f, "\r\n" );

	// output markers points
	for (int i=0; i<pModel->Children.GetCount(); i++)
		if (pModel->Children[i]->Is(FBModelMarkerOptical::TypeInfo))
		{
			FBModelMarkerOptical *pMarker = (FBModelMarkerOptical*) pModel->Children[i];
			pMarker->ExportBegin();
		}

	double pX, pY, pZ, pOcclusion;
	float scale;

	// dropAsText
	buffer.fill(0);
	cbufferPtr = buffer.data();

	lConfig.GetOrSet( "EXPORT", "dropAsText", cbufferPtr );
	char dropText = cbufferPtr[0];

	// rightCoordSys
	buffer.fill(0);
	cbufferPtr = buffer.data();

	lConfig.GetOrSet( "EXPORT", "rightCoordSys", cbufferPtr );
	char rCoordSys = cbufferPtr[0];

	// scale
	buffer.fill(0);
	sprintf_s(buffer.data(), buffer.size(), "10.0" );
	cbufferPtr = buffer.data();

	lConfig.GetOrSet( "EXPORT", "Scale", cbufferPtr );
	sscanf( cbufferPtr, "%f", &scale );

	fprintf( f, "$Points\n" );
	for (int i=startFrame; i<=stopFrame; i++)
	{
		double percent = (double)i;
		percent = percent / (stopFrame - startFrame) * 100.0;
		lProgress.Percent = (int) percent;

		fprintf( f, "%d ", i );		// output frame number

		for (int j=0; j<pModel->Children.GetCount(); j++)
			if (pModel->Children[j]->Is(FBModelMarkerOptical::TypeInfo))
			{
				FBModelMarkerOptical *pMarker = (FBModelMarkerOptical*) pModel->Children[j];
				pMarker->ExportKey(&pX, &pY, &pZ, &pOcclusion);

				// right handed system
				if (rCoordSys == '1')
				{
					double temp = pY;
					pY = -pZ;
					pZ = temp;
				}

				if (pOcclusion == 1.0)
				{
					pX = 0.0;
					pY = 0.0;
					pZ = 0.0;
					if (dropText == '1')
						fprintf( f, "DROPOUT " );
					else
						fprintf( f, "%f %f %f ", pX, pY, pZ );
				}
				else
				{
					/*
					pX = pX * 10.81;
					pY = pY * 10.81;
					pZ = pZ * 10.81;
					*/
					pX = pX * scale;
					pY = pY * scale;
					pZ = pZ * scale;

					fprintf( f, "%f %f %f ", pX, pY, pZ );
				}
			}
		fprintf( f, "\r\n" );
	}
	fprintf( f, "\n" );
	
	// finalize
	for (int i=0; i<pModel->Children.GetCount(); i++)
		if (pModel->Children[i]->Is(FBModelMarkerOptical::TypeInfo))
		{
			FBModelMarkerOptical *pMarker = (FBModelMarkerOptical*) pModel->Children[i];
			pMarker->ExportEnd();
		}

	fclose(f);
	return true;
}

#define IS_ALPHA(c) (((c) >= 'A' && (c) <= 'Z') || ((c) >= 'a' && (c) <= 'z'))
#define TO_UPPER(c) ((c) & 0xDF)

char * __cdecl strstri (const char * str1, const char * str2){
        char *cp = (char *) str1;
        char *s1, *s2;

        if ( !*str2 )
            return((char *)str1);

        while (*cp){
                s1 = cp;
                s2 = (char *) str2;

                while ( *s1 && *s2 && (IS_ALPHA(*s1) && IS_ALPHA(*s2))?!(TO_UPPER(*s1) - TO_UPPER(*s2)):!(*s1-*s2))
                        ++s1, ++s2;

                if (!*s2)
                        return(cp);

                ++cp;
        }
        return(NULL);
}

bool CSMEngine::ImportFile( const char* pFileName )
{

	FBProgress	lProgress;
	lProgress.Caption = "Import CSM";
	lProgress.Text = "importing file...";
	lProgress.Percent = 0;

	mode = csm_common;

	printf("importing file... %s\n", pFileName );

	// if successful importation, return true
	FILE *f;

	f = fopen( pFileName, "r" );
	if ( f == NULL ) return false;

	char buffer[2048];
	char ptr[80];

	int fillCount = 0;
	csm_data::marker_data	data;
	data.frame = 0;
	memset( &data.pos[0], 0, sizeof(float)*MAX_MARKERS*3 );

	{while(!feof(f) ) {
		memset(buffer,0,sizeof(char)*2048);
		fgets(buffer, 2048, f);

		switch(mode)
		{
		case csm_common:
			{
				if ( strstri(buffer, "$FirstFrame") != NULL ){
					GetTextParts(NULL);
					GetTextPart(buffer, 1, ptr);
					if (ptr) {
						sscanf( ptr, "%d", &csm.firstframe );
					}
				}
				else
				if ( strstri(buffer, "$LastFrame") != NULL || strstri(buffer, "$lasframe") != NULL )
				{
					GetTextParts(NULL);
					GetTextPart(buffer, 1, ptr);
					if (ptr) {
						sscanf( ptr, "%d", &csm.lastframe );
					}
				}
				else
				if ( strstri(buffer, "$Rate") != NULL )
				{
					GetTextParts(NULL);
					GetTextPart(buffer, 1, ptr);
					if (ptr) {
						sscanf( ptr, "%f", &csm.framerate );
					}
				}
				else
				if ( strstri(buffer, "$Order") != NULL)
				{
					mode = csm_markers;
				}
				else
				if ( strstri(buffer, "$Points") != NULL)
				{
					mode = csm_points;
					fillCount = csm.markers.GetCount() * 3;
					data.frame = csm.firstframe-1;
				}

			}break;
		case csm_markers:
			{
				if ( strstri(buffer, "$Points") != NULL)
				{
					mode = csm_points;
					fillCount = csm.markers.GetCount() * 3;
					data.frame = csm.firstframe-1;
				}
				else
				{
					int count = GetTextParts(buffer);
					for (int i=0; i<count; i++)
					{
						GetTextPart( buffer, i, ptr );
						if (ptr && isalpha(ptr[0])) {
							FBString markerName(ptr);
							csm.markers.Add( new csm_data::marker_name(markerName) );
						}
					}
				}

			}break;
		case csm_points:
			{
				float percent = (float) data.frame;
				if ((csm.lastframe - csm.firstframe) > 0)
					percent = percent / (csm.lastframe - csm.firstframe) * 100.0f;
				lProgress.Percent = (int) percent;

				int count = GetTextParts(buffer);
				int ctrlLen = csm.markers.GetCount() * 3;

				if ( ctrlLen == fillCount)
				{
					// put frame data into the points array
					if (data.frame >= csm.firstframe)
						csm.points.Add(data);
					if (data.frame == csm.lastframe)
					{
						mode = csm_common;
						break;
					}

					fillCount=0;
					// get frame count
					GetTextPart( buffer, 0, ptr );
					if (ptr) {
						sscanf( ptr, "%d", &data.frame );
					}
					// get frame positions
					for (int i=1; i<count; i++)
					{
						GetTextPart( buffer, i, ptr );
						if (ptr && ( fillCount < ctrlLen )) {
							sscanf( ptr, "%f", &data.pos[fillCount] );
							fillCount++;
						}
					}
				}
				else
				{
					// frame index
					GetTextPart( buffer, 0, ptr );
					if (ptr)
						sscanf( ptr, "%d", &data.frame );

					// get frame positions
					for (int i=1; i<count; i++)
					{
						GetTextPart( buffer, i, ptr );
						if (ptr && ( fillCount < ctrlLen )) {
							sscanf( ptr, "%f", &data.pos[fillCount] );
							fillCount++;
						}
					}
				}

			}break;
		}

	}}

	fclose(f);


	CreateOpticalModel(csm.lastframe);
	return true;
}



/************************************************
 *	Find an animation node from a given root.
 ************************************************/
FBAnimationNode *CSMEngine::FindAnimationNode( FBAnimationNode *pNode, char* pName )
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



