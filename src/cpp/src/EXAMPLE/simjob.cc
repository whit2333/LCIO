
#include "lcio.h"

#include "IO/LCWriter.h"
#include "EVENT/LCIO.h"
#include "DATA/LCFloatVec.h"
#include "DATA/LCIntVec.h"

#include "IMPL/LCEventImpl.h" 
#include "IMPL/LCRunHeaderImpl.h" 
#include "IMPL/LCCollectionVec.h"
#include "IMPL/SimCalorimeterHitImpl.h"
#include "IMPL/SimTrackerHitImpl.h"
#include "IMPL/MCParticleImpl.h" 
#include "IMPL/LCFlagImpl.h" 
#include "IMPL/LCTOOLS.h"
#include "IMPL/TPCHitImpl.h"

#include <cstdlib>
#include <iostream>
#include <sstream>


using namespace std ;
using namespace lcio ;

static const int NRUN = 10 ;
static const int NEVENT = 10 ; // events
static const int NMCPART = 10 ;  // mc particles per event
static const int NHITS = 50 ;  // calorimeter hits per event

static string FILEN = "simjob.slcio" ;


/** Simple test program to demonstrate writing of data with lcio.
 */

int main(int argc, char** argv ){
  
  try{
    // create sio writer
    LCWriter* lcWrt = LCFactory::getInstance()->createLCWriter() ;
    
    if( argc > 1 ) { FILEN = argv[1] ; }
    
    try{  lcWrt->open( FILEN ) ;
    }
    catch( IOException& e ){    
      cout << e.what() << endl ;
      return 0 ;
    }
    
    // loop over runs
    for(int rn=0;rn<NRUN;rn++){
      
      LCRunHeaderImpl* runHdr = new LCRunHeaderImpl ; 
      runHdr->setRunNumber( rn ) ;
      
      string detName("D09TileHcal")  ;
      runHdr->setDetectorName( detName ) ;
      
      stringstream description ; 
      description << " run: " << rn <<" just for testing lcio  - no physics !" ;
      runHdr->setDescription( description.str()  ) ;
      
      string ecalName("ECAL007") ;
      runHdr->addActiveSubdetector( ecalName ) ;
      
      string tpcName("TPC4711") ;
      runHdr->addActiveSubdetector( tpcName ) ;
      
      lcWrt->writeRunHeader( runHdr ) ;
      
      // EventLoop - create some events and write them to the file
      for(int i=0;i<NEVENT;i++){
	
	// we need to use the implementation classes here 
	LCEventImpl*  evt = new LCEventImpl() ;
	
	
	evt->setRunNumber(  rn   ) ;
	evt->setEventNumber( i ) ;
	evt->setDetectorName( detName ) ;
	
	// create and add some mc particles 
	LCCollectionVec* mcVec = new LCCollectionVec( LCIO::MCPARTICLE )  ;
	

	MCParticleImpl* mom = new MCParticleImpl ;
	mom->setPDG( 1  ) ;
	float p0[3] = { 0. , 0. , 1000. } ;
	mom->setMomentum( p0 ) ;
	mom->setMass( 3.01 ) ;

	for(int j=0;j<NMCPART;j++){

	  MCParticleImpl* mcp = new MCParticleImpl ;

	  mcp->setPDG( 1000 * (j+1)  ) ;
	  float p[3] = { j*1. , 4./1024. , 8./1024. } ;
	  mcp->setMomentum( p ) ;
	  mcp->setMass( .135 ) ;

	  // create and add some daughters
	  for(int k=0;k<3;k++){
	    MCParticleImpl* d1 = new MCParticleImpl ;

	    d1->setPDG( 1000 * (j+1) + 100 * (k+1)  ) ;
	    float pd1[3] = { k*1. , 4.1 , 8.1 } ;
	    d1->setMomentum( pd1 ) ;
	    d1->setMass( .135 ) ;

	    for(int l=0;l<2;l++){
	      MCParticleImpl* d2 = new MCParticleImpl ;
	      
	      d2->setPDG( 1000 * (j+1) + 100 * (k+1) + 10 *  (l+1)  ) ;
	      float pd2[3] = { l*1. , 0.41 , 4.1 } ;
	      d2->setMomentum( pd2 ) ;
	      d2->setMass( .135 ) ;

	      d2->addParent( d1 );
	      mcVec->push_back( d2 ) ;
	    }
	    d1->addParent( mcp );
	    mcVec->push_back( d1 ) ;
	  }
	  
	  mcp->addParent( mom );
	  mcVec->push_back( mcp ) ;
	}
	mcVec->push_back( mom ) ;
	
	// now add some calorimeter hits
	LCCollectionVec* calVec = new LCCollectionVec( LCIO::SIMCALORIMETERHIT )  ;
      
	// set flag for long format (including position )
	// and PDG 
	LCFlagImpl chFlag(0) ;
	chFlag.setBit( LCIO::CHBIT_LONG ) ;
	chFlag.setBit( LCIO::CHBIT_PDG ) ;
	calVec->setFlag( chFlag.getFlag()  ) ;
	
	
	for(int j=0;j<NHITS;j++){
	  
	  SimCalorimeterHitImpl* hit = new SimCalorimeterHitImpl ;
	  
	  hit->setEnergy( 3.1415 * rand()/RAND_MAX   )  ;
	  
	  float pos[3] = { 1.1* rand()/RAND_MAX , 2.2* rand()/RAND_MAX , 3.3* rand()/RAND_MAX } ;
	  
	  hit->setPosition( pos ) ;
	  
	  calVec->push_back( hit ) ;
	  
	  // assign the hits randomly to MC particles
	  float rn =  .99999*rand()/RAND_MAX ;
	  int mcIndx = static_cast<int>( NMCPART * rn ) ;
	  
	  // in order to access a MCParticle,  we need a dynamic cast as the 
	  // LCCollection returns an LCIOObject - this is like vectors in Java 
	  hit->addMCParticleContribution(  dynamic_cast<MCParticle*>(mcVec->getElementAt( mcIndx )) , 
					   0.314159, 0.1155 ) ; // no pdg
	  
	}
	
	// -------- data can be modified as long as is not not made persistent --------

	for(int j=0;j<NHITS;j++){
	  SimCalorimeterHitImpl* existingHit 
	    = dynamic_cast<SimCalorimeterHitImpl*>( calVec->getElementAt(j) ) ; // << Ok now

 	  //	    = dynamic_cast<SimCalorimeterHitImpl*>( (*calVec)[j] ) ;  // << not needed 
	  
	  existingHit->addMCParticleContribution( dynamic_cast<MCParticle*>
						  (mcVec->getElementAt(0)), 
						  0.1, 0. ) ;
	}

	// and finally some tracker hits
	// with some user extensions (4 floats and 2 ints) per track:
	// we just need to create parallel collections of float and int vectors
	LCCollectionVec* trkVec = new LCCollectionVec( LCIO::SIMTRACKERHIT )  ;
	LCCollectionVec* extFVec = new LCCollectionVec( LCIO::LCFLOATVEC )  ;
	LCCollectionVec* extIVec = new LCCollectionVec( LCIO::LCINTVEC )  ;
	
	for(int j=0;j<NHITS;j++){
	  
	  SimTrackerHitImpl* hit = new SimTrackerHitImpl ;
	  LCFloatVec* extF = new LCFloatVec ;
	  LCIntVec*   extI = new LCIntVec ;
	  
	  hit->setdEdx( 30e-9 ) ; 

	  double pos[3] = { 1.1* rand()/RAND_MAX , 2.2* rand()/RAND_MAX , 3.3* rand()/RAND_MAX } ;
	  
	  hit->setPosition( pos ) ; 
	  
	  // assign the hits randomly to MC particles
	  float rn =  .99999*rand()/RAND_MAX ;
	  int mcIndx = static_cast<int>( NMCPART * rn ) ;
	  
	  hit->setMCParticle( dynamic_cast<MCParticle*>(mcVec->getElementAt( mcIndx ) ) ) ;
	  
	  
	  // fill the extension vectors (4 floats, 2 ints)
	  extF->push_back( 3.14159 ) ;  
	  for(int k=0;k<3;k++) extF->push_back(  pos[k] * 0.1  ) ;

	  extI->push_back( 123456789 ) ;
	  extI->push_back( mcIndx ) ;

	  // add the hit and the extensions to their corresponding collections
	  trkVec->push_back( hit ) ;
	  extFVec->push_back( extF ) ;
	  extIVec->push_back( extI ) ;
	}
	
	
	// add all collections to the event
	evt->addCollection( mcVec , "MCParticle" ) ;
	evt->addCollection( calVec , ecalName ) ;
	evt->addCollection( trkVec , tpcName ) ;
	evt->addCollection( extFVec , tpcName+"UserFloatExtension" ) ;
	evt->addCollection( extIVec , tpcName+"UserIntExtension" ) ;
	
	// test: add a collection for one event only:
	if( rn == NRUN-1 && i == 0 ) { // first event o last run
	  LCCollectionVec* addExtVec = new LCCollectionVec( LCIO::LCFLOATVEC )  ;
	  LCFloatVec* addExt = new LCFloatVec ;
	  addExt->push_back( 1. );
	  addExt->push_back( 2. );
	  addExt->push_back( 3. );
	  addExt->push_back( 4. );
	  addExtVec->push_back( addExt ) ;
	  evt->addCollection( addExtVec , "AdditionalExtension" ) ;
	}

	// even though this is a simjob we can store 'real data' objects :)
	// --- for example we can store TPC hits ------------

	LCCollectionVec* TPCVec = new LCCollectionVec( LCIO::TPCHIT )  ;

	 
	bool storeRawData = true ;

	LCFlagImpl tpcFlag(0) ;
	if(  storeRawData )  // if we want to store the raw data we need to set the flag
	  tpcFlag.setBit( LCIO::TPCBIT_RAW ) ;
	TPCVec->setFlag( tpcFlag.getFlag()  ) ;
	
	for(int j=0;j<NHITS;j++){
	  
	  TPCHitImpl* tpcHit = new TPCHitImpl ;
	  
	  tpcHit->setCellID( j ) ;
	  tpcHit->setTime( 0.1234567 ) ;
	  tpcHit->setCharge( 3.14159 ) ;
	  tpcHit->setQuality(  0xbad ) ;

	  if(  storeRawData ) {
	    int rawData[10] ;
	    // fill some random numbers 
	    int size =   int( (double(rand()) / RAND_MAX ) * 10 )  ;   
	    for(int k=0;k<size;k++){
	      rawData[k] = int( (double(rand()) / RAND_MAX ) * INT_MAX ) ;   
	    }

	    tpcHit->setRawData( rawData , size ) ;
	  }

	  TPCVec->push_back( tpcHit ) ;
	}	
	evt->addCollection( TPCVec , "TPCRawFADC" ) ;
	//--------------  all for TPC --------------------


	// write the event to the file
	lcWrt->writeEvent( evt ) ;
	
	// dump the event to the screen 
	LCTOOLS::dumpEvent( evt ) ;

	// ------------ IMPORTANT ------------- !
	// we created the event so we need to delete it ...
	delete evt ;
	// -------------------------------------

	if( ! (i%100) ) cout << ". " << flush  ;
	
      } // evt loop

      delete runHdr ;

    } // run loop
    
    cout << endl 
	 << " created  " << NRUN << " runs with  " << NRUN*NEVENT << " events"  
	 << endl << endl ;
    
    
    
    lcWrt->close() ;
  
  } catch( Exception& ex){

    cout << " an excpetion occured: " << endl ;
    cout << "  " << ex.what() << endl ;
    return 1 ;
  }

  return 0 ;
}

