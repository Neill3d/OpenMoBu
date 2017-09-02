
/////////////////////////////////////////////////////////////////////////////////////////
//
// Licensed under the "New" BSD License. 
//		License page - https://code.google.com/p/motioncodelibrary/wiki/LICENSE
//
// GitHub repository - https://github.com/Neill3d/MoBu
//
// Author Sergey Solohin (Neill3d) 2014
//  e-mail to: s@neill3d.com
//		www.neill3d.com
/////////////////////////////////////////////////////////////////////////////////////////

#include "helper.h"

void
DisplayErrorText(
    DWORD dwLastError
    )
{
    HMODULE hModule = NULL; // default to system source
    LPSTR MessageBuffer;
    DWORD dwBufferLength;

    DWORD dwFormatFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_IGNORE_INSERTS |
        FORMAT_MESSAGE_FROM_SYSTEM ;

    //
    // Call FormatMessage() to allow for message 
    //  text to be acquired from the system 
    //  or from the supplied module handle.
    //

    if(dwBufferLength = FormatMessageA(
        dwFormatFlags,
        hModule, // module to get message from (NULL == system)
        dwLastError,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // default language LANG_RUSSIAN
		//MAKELANGID(LANG_RUSSIAN, SUBLANG_SYS_DEFAULT), // default language 
        (LPSTR) &MessageBuffer,
        0,
        NULL
        ))
    {
        DWORD dwBytesWritten;

        //
        // Output message string on stderr.
        //
        WriteFile(
            GetStdHandle(STD_ERROR_HANDLE),
            MessageBuffer,
            dwBufferLength,
            &dwBytesWritten,
            NULL
            );

        //
        // Free the buffer allocated by the system.
        //
        LocalFree(MessageBuffer);
    }

    //
    // If we loaded a message source, unload it.
    //
    if(hModule != NULL)
        FreeLibrary(hModule);
}

void AdditiveCalculate()
{
	FBApplication	lApp;
	FBSystem		lSystem;
	HFBCharacter	lChar = lApp.CurrentCharacter;
	FBPlayerControl	lPlayer;

	if (lChar) {
		// set keys for each frame
		FBPlayerControl		lPlayer;

		FBTime lTemp;
		lTemp.SetTime( 0,0,0, 60 );
		lPlayer.Goto(lTemp);

		int n=0;
		while (n<10) {
			lSystem.Scene->Evaluate();

			int startIndex, stopIndex;
			startIndex = kFBHipsNodeId;
			stopIndex = kFBLastNodeId;

			while (startIndex < stopIndex)
			{
				HFBModel lModel = lChar->GetModel(FBBodyNodeId(startIndex));

				if (lModel)
				{
					FBVector3d	lRotDst, lRotSrc, lRotRes;
					FBQuaternion lQuatDst, lQuatSrc, lQuatRes;
						
					// get dst rotation
					lSystem.CurrentTake = lSystem.Scene->Takes[0];
					lSystem.Scene->Evaluate();
				
					lModel->GetVector( lRotDst, kModelRotation, false );

					// get dst rotation
					lSystem.CurrentTake = lSystem.Scene->Takes[1];
					lSystem.Scene->Evaluate();

					lModel->GetVector( lRotSrc, kModelRotation, false );


					// result put in take 003
					lSystem.CurrentTake = lSystem.Scene->Takes[2];

					// calculate subtract of this
					FBRotationToQuaternion( lQuatDst, lRotDst );
					FBRotationToQuaternion( lQuatSrc, lRotSrc );
					FBQSub( lQuatRes, lQuatDst, lQuatSrc );
					FBQuaternionToRotation( lRotRes, lQuatRes );

					HFBAnimationNode	lAnimNode = lModel->AnimationNode;
					HFBAnimationNode	lTransNode = lAnimNode->Nodes.Find("Lcl Rotation");

					FBTime lTime = lSystem.LocalTime;

					HFBAnimationNode lXNode	= lTransNode->Nodes.Find("X");
					lXNode->KeyAdd( &lRotRes[0] );
					
					lXNode	= lTransNode->Nodes.Find("Y");
					lXNode->KeyAdd( &lRotRes[1] );

					lXNode	= lTransNode->Nodes.Find("Z");
					lXNode->KeyAdd( &lRotRes[2] );
				}

				startIndex++;
			}

			n++;
			lPlayer.StepForward();
		}
	}
}

void GeometryReplace()
{
	HFBModel srcModel, dstModel;

	srcModel = FBFindModelByName( "srcModel" );
	dstModel = FBFindModelByName( "dstModel" );


	FBCluster	srcCluster(srcModel);
	FBCluster	dstCluster(dstModel);
	FBString	linkName;
	HFBModel	linkModel;
	int numLinks, numVerts, vertIndex;
	double vertWeight;

	numLinks = dstCluster.LinkGetCount();

	for (int i=0; i<numLinks; i++)
	{
		dstCluster.ClusterBegin(i);	//set the current cluster index
		srcCluster.ClusterBegin(i);

		linkName = dstCluster.LinkGetName(i);
		linkModel = dstCluster.LinkGetModel(i);

		srcCluster.LinkSetName( linkName, i );
		srcCluster.LinkSetModel( linkModel );

		/*
		numVerts = dstCluster.VertexGetCount();
		srcCluster.VertexClear();
		for (int j=0; j<numVerts; j++)
		{
			vertIndex = dstCluster.VertexGetNumber(j);		// Using the current cluster index
			vertWeight = dstCluster.VertexGetWeight(j);	// Using the current cluster index

			if (vertWeight > 0.0)
			{
				srcCluster.VertexAdd( vertIndex, vertWeight );
			}
		}
		*/

		dstCluster.ClusterEnd();
		srcCluster.ClusterEnd();
	}
}