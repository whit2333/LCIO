#include "SIO/SIOCalHitHandler.h"

#include "SIO/LCSIO.h"

#include "EVENT/MCParticle.h"
#include "EVENT/CalorimeterHit.h"
#include "IOIMPL/CalorimeterHitIOImpl.h"

#include "SIO_functions.h"
#include "SIO_block.h"

using namespace EVENT ;
using namespace IMPL ;
using namespace IOIMPL ;


namespace SIO{
    
  unsigned int SIOCalHitHandler::read(SIO_stream* stream, 
				      LCObject** objP,
				      unsigned int flag,
				      unsigned int vers ){
    unsigned int status ; 
	
    // create a new object :
    CalorimeterHitIOImpl* hit  = new CalorimeterHitIOImpl ;
    *objP = hit ;
	
    SIO_DATA( stream ,  &(hit->_cellID0) , 1  ) ;
    SIO_DATA( stream ,  &(hit->_cellID1) , 1  ) ;
    SIO_DATA( stream ,  &(hit->_energy) , 1  ) ;
    SIO_DATA( stream ,  hit->_position  , 3 ) ;
    
//     cout << " reading hit from file :" 
// 	 << hit->_cellID0 << "  " 
// 	 << hit->_cellID1 << "  " 
// 	 << hit->_energy << "  " 
// 	 << hit->_position << "  "  << endl ;


    // read MCContributions
    int nCon ;
    SIO_DATA( stream ,  &nCon , 1  ) ;

//     cout << "   nCOn :  " <<  nCon << endl ;


    for(int i=0; i< nCon ; i++){

      MCParticleCont* mcCon = new MCParticleCont ;
      SIO_PNTR( stream , &(mcCon->Particle)  ) ;
      SIO_DATA( stream , &(mcCon->Energy) , 1 ) ;
      SIO_DATA( stream , &(mcCon->Time)   , 1 ) ;
      SIO_DATA( stream , &(mcCon->PDG)    , 1 ) ;

      hit->_vec.push_back(  mcCon  );
    }

    // if we want to point at calo hits we need to add a ptag:
    //  SIO_PTAG( stream , dynamic_cast<const CalorimeterHit*>(hit) ) ;
	
    return ( SIO_BLOCK_SUCCESS ) ;
	
  }
    
    
  unsigned int SIOCalHitHandler::write(SIO_stream* stream, 
				       const LCObject* obj,
				       unsigned int flag ){
    
    unsigned int status ; 
	
    // this is where we gave up type safety in order to
    // simplify the API and the implementation
    // by having a common collection of objects
    
    const CalorimeterHit* hit = dynamic_cast<const CalorimeterHit*>(obj)  ;
    
    LCSIO_WRITE( stream, hit->getCellID0()  ) ;
    LCSIO_WRITE( stream, hit->getCellID1()  ) ;
    LCSIO_WRITE( stream, hit->getEnergy()  ) ;
    // as SIO doesn't provide a write function with const arguments
    // we have to cast away the constness 
    float* pos = const_cast<float*> ( hit->getPosition() ) ; 
    SIO_DATA( stream,  pos , 3 ) ;
    
    // now the MCParticle contributions
    int nMC = hit->getNMCParticles() ;
    SIO_DATA( stream,  &nMC , 1 ) ;

    for(int i=0; i<nMC ;i++){

      const MCParticle* part = hit->getParticleCont(i)  ;
      SIO_PNTR( stream , &part ) ;
      
      LCSIO_WRITE( stream, hit->getEnergyCont(i)  ) ;
      LCSIO_WRITE( stream, hit->getTimeCont(i)  ) ;
      LCSIO_WRITE( stream, hit->getPDGCont(i)  ) ;
      
    }
    //  SIO_PTAG( stream , dynamic_cast<const CalorimeterHit*>(hit) ) ;

    return ( SIO_BLOCK_SUCCESS ) ;
    
  }
  
}; // namespace