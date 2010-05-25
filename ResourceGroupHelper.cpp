// taken from http://www.ogre3d.org/forums/viewtopic.php?f=5&t=46787&start=25#p329272
#include "stdafx.h"
#include "ResourceGroupHelper.h"
#include "OgreResourceGroupManager.h"
#include "OgreLogManager.h"
#include "OgreRoot.h"
#include "OgreMovableObject.h"
#include "OgreMaterialManager.h"
#include "OgreEntity.h"
#include "OgreSubEntity.h"
#include "OgreBillBoardChain.h"
#include "OgreBillBoardSet.h"
#include "OgreOverlayElement.h"
#include "OgreOverlay.h"
#include "OgreOverlayManager.h"
#include "OgreOverlayContainer.h"

ResourceGroupHelper::UpdateMaterialRenderableVisitor::UpdateMaterialRenderableVisitor():
Ogre::Renderable::Visitor()
{
}

void ResourceGroupHelper::UpdateMaterialRenderableVisitor::visit(
   Ogre::Renderable *rend, Ogre::ushort lodIndex, bool isDebug, Ogre::Any *pAny)
{
   const Ogre::MaterialPtr mat = rend->getMaterial();
   if(!mat.isNull())
   {
      std::string newMatName = mat->getName();
      Ogre::MaterialPtr newMat = Ogre::MaterialManager::getSingleton().getByName(newMatName);
      if(newMat.isNull())
      {
         // this can happen if there was error during the reloading of the material.
         // in that case, we keep the ancient one.
         // Ogre::LogManager::getSingleton().logMessage(newMatName+" : new material is null!");
         return;
      }

      // unfortunately, the renderable gives access only to a const MaterialPtr.
      // and there is no 'setMaterial' or 'setMaterialName' method on renderables.
      // so I have to try to down cast with known classes...
      {   
         Ogre::SubEntity* lRend = dynamic_cast<Ogre::SubEntity*>(rend);
         if(lRend){lRend->setMaterialName(newMatName);return;} 
      }
      {
         Ogre::SimpleRenderable* lRend = dynamic_cast<Ogre::SimpleRenderable*>(rend);
         if(lRend){lRend->setMaterial(newMatName);return;} 
      }
      {
         Ogre::ShadowRenderable* lRend = dynamic_cast<Ogre::ShadowRenderable*>(rend);
         if(lRend){lRend->setMaterial(newMat);return;} 
      }
      {   
         Ogre::BillboardChain* lRend = dynamic_cast<Ogre::BillboardChain*>(rend);
         if(lRend){lRend->setMaterialName(newMatName);return;} 
      }
      {   
         Ogre::BillboardSet* lRend = dynamic_cast<Ogre::BillboardSet*>(rend);
         if(lRend){lRend->setMaterialName(newMatName);return;} 
      }
      {   
         Ogre::OverlayElement* lRend = dynamic_cast<Ogre::OverlayElement*>(rend);
         if(lRend){lRend->setMaterialName(newMatName);return;} 
      }
   }else{
      // was there for debug...
      // Ogre::LogManager::getSingleton().logMessage("material of renderable is null!");
   }
}

ResourceGroupHelper::ResourceGroupHelperLogListener::ResourceGroupHelperLogListener():
Ogre::LogListener(),mKeptMessages()
{
   Ogre::LogManager* logMgr = Ogre::LogManager::getSingletonPtr();
   if(logMgr)
   {
      bool logExist = true;
      try{logMgr->getDefaultLog();}catch(Ogre::Exception&)
      {
         logExist = false;
      }
      if(logExist)
      {
         logMgr->getDefaultLog()->addListener(this);
      }
   }
}

ResourceGroupHelper::ResourceGroupHelperLogListener::~ResourceGroupHelperLogListener()
{
   Ogre::LogManager* logMgr = Ogre::LogManager::getSingletonPtr();
   if(logMgr)
   {
      bool logExist = true;
      try{logMgr->getDefaultLog();}catch(Ogre::Exception&)
      {
         logExist = false;
      }
      if(logExist)
      {
         logMgr->getDefaultLog()->removeListener(this);
      }
   }
}

bool ResourceGroupHelper::ResourceGroupHelperLogListener::areMessagesKept()
{
   return mKeptMessages.peek()==EOF;
}

std::string ResourceGroupHelper::ResourceGroupHelperLogListener::getKeptMessages()
{
   return mKeptMessages.str();
}

void ResourceGroupHelper::ResourceGroupHelperLogListener::clearKeptMessages()
{
   mKeptMessages.str(std::string());
}

void ResourceGroupHelper::ResourceGroupHelperLogListener::messageLogged
   (const Ogre::String &message, Ogre::LogMessageLevel lml, bool maskDebug, const Ogre::String &logName)
{
   Ogre::String copy = message;
   Ogre::StringUtil::toLowerCase(copy);
   Ogre::String pattern1 = "*error*";
   Ogre::String pattern2 = "*exception*";
   bool lErrorfound = Ogre::StringUtil::match(message,pattern1,true) || Ogre::StringUtil::match(message,pattern2,true);
   if(lErrorfound)
   {
      mKeptMessages<<message;
   }
}


ResourceGroupHelper::ResourceGroupHelper(void):
mRessourceGroupModificationTimes()
{
   // reloading the default resource group is a bad idea.
   // lets make it harder, by giving it a big modification time.
   time_t veryBig = 1;
   {
      int nbBytes = sizeof(time_t);
      for(int i = 0; i<(nbBytes-1)*8;i++)
      {
         veryBig+=2*veryBig;
      }
   }
   mRessourceGroupModificationTimes[Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME] = veryBig;
}

ResourceGroupHelper::~ResourceGroupHelper(void)
{
}

std::vector<ResourceGroupHelper::ArchivePathAndType> ResourceGroupHelper::getAllPathAndTypesNames(const std::string& pResourceGroupName)
{
   // note : unfortunately no access to the order in which path where loaded
   Ogre::FileInfoListPtr fli_dirs = Ogre::ResourceGroupManager::getSingleton().listResourceFileInfo(pResourceGroupName,true);

   std::vector<ResourceGroupHelper::ArchivePathAndType> lDirectoryInfos;

   if(!fli_dirs.isNull())
   {
      Ogre::FileInfoList::iterator itFliD = fli_dirs->begin();
      Ogre::FileInfoList::iterator itFliDEnd = fli_dirs->end();
      for(; itFliD!=itFliDEnd;itFliD++)
      {
         Ogre::FileInfo& lFinfoD = (*itFliD);
         if(lFinfoD.archive)
         {
            ResourceGroupHelper::ArchivePathAndType arch;
            arch.first = lFinfoD.path + lFinfoD.archive->getName();
            arch.second = lFinfoD.archive->getType();
            lDirectoryInfos.push_back(arch);
         }
      }
   }
   return lDirectoryInfos;
}



bool ResourceGroupHelper::reloadAResourceGroup(const std::string& pResourceGroupName,const std::vector<ResourceGroupHelper::ArchivePathAndType>& pLocationToAdd)
{
   if(!resourceGroupExist(pResourceGroupName))
   {
      // not present. something wrong.
      return false;
   }

   Ogre::ResourceGroupManager& resGroupMgr = Ogre::ResourceGroupManager::getSingleton();
   resGroupMgr.destroyResourceGroup(pResourceGroupName);

   if(resourceGroupExist(pResourceGroupName))
   {
      // still present. something wrong.
      return false;
   }

   resGroupMgr.createResourceGroup(pResourceGroupName);

   std::vector<ResourceGroupHelper::ArchivePathAndType>::const_iterator iterLoc= pLocationToAdd.begin();
   std::vector<ResourceGroupHelper::ArchivePathAndType>::const_iterator iterLocEnd= pLocationToAdd.end();
   for(;iterLoc!=iterLocEnd;iterLoc++)
   {
      // add the location in the resourceGroup
      resGroupMgr.addResourceLocation(iterLoc->first,iterLoc->second,pResourceGroupName,false);
   }
   resGroupMgr.initialiseResourceGroup(pResourceGroupName);
   return true;
}



bool ResourceGroupHelper::reloadAResourceGroupWithoutDestroyingIt(const std::string& pResourceGroupName)
{
   if(!resourceGroupExist(pResourceGroupName))
   {
      // not present. something wrong.
      return false;
   }
   Ogre::ResourceGroupManager& resGroupMgr = Ogre::ResourceGroupManager::getSingleton();
   resGroupMgr.clearResourceGroup(pResourceGroupName);
   resGroupMgr.initialiseResourceGroup(pResourceGroupName);
   return true;
}


bool ResourceGroupHelper::resourceGroupExist(const std::string& pResourceGroupName)
{
   bool lIsPresent = false;
   Ogre::ResourceGroupManager& resGroupMgr = Ogre::ResourceGroupManager::getSingleton();
   Ogre::StringVector lAllResourceGroups = resGroupMgr.getResourceGroups();
   Ogre::StringVector::iterator iter = lAllResourceGroups.begin();
   Ogre::StringVector::iterator iterEnd = lAllResourceGroups.end();
   for(;iter!=iterEnd;iter++)
   {
      if((*iter) == pResourceGroupName)
      {
         lIsPresent = true;
      }
   }
   return lIsPresent;
}


void ResourceGroupHelper::updateOnEveryRenderable()
{

   //1/ get all the available object type (entity, light, user defined types ...)
   std::vector<std::string> allAvailableTypes; 
   Ogre::Root::MovableObjectFactoryIterator iterFactory = Ogre::Root::getSingleton().getMovableObjectFactoryIterator();
   for(;iterFactory.hasMoreElements();)
   {
      Ogre::MovableObjectFactory* factory = iterFactory.getNext();
      allAvailableTypes.push_back(factory->getType());
   }

   UpdateMaterialRenderableVisitor lRenderableVisitor;

   //2/ for each scene manager, lets visit renderables!
   // unfortunately that does not cover all renderables type... (overlays...)
   Ogre::SceneManagerEnumerator::SceneManagerIterator iterSceneManager = Ogre::Root::getSingleton().getSceneManagerIterator();
   for(;iterSceneManager.hasMoreElements();)
   {
      Ogre::SceneManager * scMgr = iterSceneManager.getNext();

      std::vector<std::string>::iterator iterMovableType = allAvailableTypes.begin();
      std::vector<std::string>::iterator iterMovableTypeEnd = allAvailableTypes.end();
      for(;iterMovableType!=iterMovableTypeEnd;iterMovableType++)
      {
         Ogre::SceneManager::MovableObjectIterator iterMovable = scMgr->getMovableObjectIterator(*iterMovableType);
         for(;iterMovable.hasMoreElements();)
         {
            Ogre::MovableObject * movable = iterMovable.getNext();
            movable->visitRenderables(&lRenderableVisitor,false);
         }
      }
   }

   // 3 / visit overlays!
   {
      Ogre::OverlayManager::OverlayMapIterator iterOverlay = Ogre::OverlayManager::getSingleton().getOverlayIterator();
      for(;iterOverlay.hasMoreElements();)
      {
         Ogre::Overlay* lOverlay = iterOverlay.getNext();
         // get the first level of OverlayContainer in the Overlay
         Ogre::Overlay::Overlay2DElementsIterator iterOverlayElem = lOverlay->get2DElementsIterator();
         for(;iterOverlayElem.hasMoreElements();)
         {
            Ogre::OverlayContainer * lOverlayCont = iterOverlayElem.getNext();
            visitRecursivelyRenderablesFrom(lOverlayCont,lRenderableVisitor, false);
         }
      }
   }
}


void ResourceGroupHelper::visitRecursivelyRenderablesFrom(Ogre::OverlayContainer* pOverlayContainer, Ogre::Renderable::Visitor& pVisitor, bool debugRenderable)
{
   // call on 'this'
   pOverlayContainer->visitRenderables(&pVisitor,false);
   
   // call on 'leaf' (cf composite pattern)
   {
      Ogre::OverlayContainer::ChildIterator childIter = pOverlayContainer->getChildIterator();
      for(;childIter.hasMoreElements();)
      {
         Ogre::OverlayElement* lOverElem = childIter.getNext();
         lOverElem->visitRenderables(&pVisitor,false);
      }
   }

   // call on 'not-leaf' (cf composite pattern)
   {
      Ogre::OverlayContainer::ChildContainerIterator childContainerIter = pOverlayContainer->getChildContainerIterator();
      for(;childContainerIter.hasMoreElements();)
      {
         Ogre::OverlayContainer * childContainer = childContainerIter.getNext();
         visitRecursivelyRenderablesFrom(childContainer, pVisitor,debugRenderable);
      }
   }
}

time_t ResourceGroupHelper::getLatestModificationTime(const std::string& pResourceGroupName)
{
   time_t result(0);

   Ogre::ResourceGroupManager& rgMgr = Ogre::ResourceGroupManager::getSingleton();
   Ogre::FileInfoListPtr fli_files = rgMgr.listResourceFileInfo(pResourceGroupName,false);
   if(fli_files.isNull())
   {
      // something went wrong (example : no files)!
      return result;
   }

   // for each file, we check the modification date.
   // we keep the latest one.
   Ogre::FileInfoList::iterator iterFiles = fli_files->begin();
   Ogre::FileInfoList::iterator iterFilesEnd = fli_files->end();
   for(;iterFiles!=iterFilesEnd;iterFiles++)
   {
      Ogre::FileInfo& file = *iterFiles;
      if(file.archive)
      {
         time_t modifTime = file.archive->getModifiedTime(file.filename);
         if(result < modifTime)
         {
            result = modifTime;
         }
      }
   }
   return result;
}

bool ResourceGroupHelper::checkTimeAndReloadIfNeeded(const std::string& pResourceGroupName, std::string &pOutLoggedMessages, bool useLog)
{
   bool result = false;

   // 1/ get last modification time.
   time_t lastModificationTime = getLatestModificationTime(pResourceGroupName);

   std::map<std::string, time_t >::iterator iterInfoTime = mRessourceGroupModificationTimes.find(pResourceGroupName);
   if(iterInfoTime!=mRessourceGroupModificationTimes.end())
   {
      if(iterInfoTime->second < lastModificationTime)
      {
         // update the value
         iterInfoTime->second = lastModificationTime;
         
         Ogre::LogManager::getSingleton().logMessage("----- Reloading materials -----");
         // use log if needed
         if(useLog)
         {
            // constructor register itself, and destructor unregister itself
            ResourceGroupHelperLogListener lLogListener;
            // try to reload
            reloadAResourceGroupWithoutDestroyingIt(pResourceGroupName);
            pOutLoggedMessages = lLogListener.getKeptMessages();
         }else{
            // try to reload
            reloadAResourceGroupWithoutDestroyingIt(pResourceGroupName);
         }

         // update the material of reachable renderables
         updateOnEveryRenderable();

         result = true;
      }
   }else{
      mRessourceGroupModificationTimes[pResourceGroupName] = lastModificationTime;
   }
   return result;
}