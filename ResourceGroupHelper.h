// taken from http://www.ogre3d.org/forums/viewtopic.php?f=5&t=46787&start=25#p329272

#ifndef RESOURCEGROUPHELPER_H
#define RESOURCEGROUPHELPER_H

#include <utility>
#include <string>
#include <vector>
#include <OgreRenderable.h>
#include <OgreLog.h>
///\brief a fake class with different useful methods for manipulating Resourcegroups.
/// please note that it was not tested with background loading.

class ResourceGroupHelper
{
private:

   /// \brief anti copy constructor (no use)
   ResourceGroupHelper(const ResourceGroupHelper&);
   /// \brief anti affector (no use)
   ResourceGroupHelper& operator=(const ResourceGroupHelper&);

   /// \brief a helper method to visit all overlay
   /// it uses a recursive call
   void visitRecursivelyRenderablesFrom(Ogre::OverlayContainer* pOverlayContainer, Ogre::Renderable::Visitor& pVisitor, bool debugRenderable = false);

   ///\brief a map [key : nameOfTheRessourceGroup]/[value : last modification time on the hdd from the files of the ResourceGroup]
   std::map<std::string, time_t > mRessourceGroupModificationTimes;

   // ------ helper classes
   
   /// \brief this visitor will be used to set the material on known renderable that allow this operation.
   /// for other user class renderable, must be tweaked/changed
   class UpdateMaterialRenderableVisitor : public Ogre::Renderable::Visitor
   {
   private:
      UpdateMaterialRenderableVisitor(const UpdateMaterialRenderableVisitor&);///<\brief anti copyconstructor
      UpdateMaterialRenderableVisitor& operator=(const UpdateMaterialRenderableVisitor&);///<\brief anti affector
   public: 
      /// \brief default constructor
      UpdateMaterialRenderableVisitor();
      /// \brief called for each renderable
      virtual void visit(Ogre::Renderable *rend, Ogre::ushort lodIndex, bool isDebug, Ogre::Any *pAny=0); 
   };
   
   ///\brief this class will listen to the log.
   /// it will check if there are "errors" or "exception" send to the listener
   /// and allows to keep or not the messages.
   class ResourceGroupHelperLogListener : public Ogre::LogListener
   {
   private:
      ResourceGroupHelperLogListener(const ResourceGroupHelperLogListener&);///<\brief anti copyconstructor
      ResourceGroupHelperLogListener& operator=(const ResourceGroupHelperLogListener&);///<\brief anti affector
      std::stringstream mKeptMessages;///<\brief interesting messages that we choose to keep
   public:
      /// \brief the constructor
      ResourceGroupHelperLogListener();
      ///\brief the destructor de-register itself from the log
      ~ResourceGroupHelperLogListener();
      /// \brief called for each message
      virtual void messageLogged(const Ogre::String &message, Ogre::LogMessageLevel lml, bool maskDebug, const Ogre::String &logName);
      /// \brief get a copy of kept messages
      std::string getKeptMessages();
      ///\brief tells if mKeptMessages is empty or not
      bool areMessagesKept();
      ///\brief clear the kept messages
      void clearKeptMessages();
   };

public:
   /// \brief default constructor
   ResourceGroupHelper(void);
   
   /// \brief destructor
   ~ResourceGroupHelper(void);

   /// \brief : a path + the archive type.
   typedef std::pair<std::string,std::string> ArchivePathAndType;

   /// \brief get a vector with directory path and corresponding type.
   /// can be useful if you want to reload some Resourcegroup.
   /// note : does not check the config file.
   /// \param the name of the resourcegroup
   /// \warning there is no garanty that the path order will be the same than during the first loading.
   std::vector<ArchivePathAndType> getAllPathAndTypesNames(const std::string& pResourceGroupName);

   /// \brief tries to suppress a Resourcegroup and reload it completely using the provided ordered locations.
   /// \param the name of the resourcegroup
   /// \param the path and types of the archives to be loaded, and in the order to be loaded
   /// it will not work correctly if some elements of the Resourcegroup are still used.
   /// \return true if the Resourcegroup was found and correctly destroy, false otherwise.
   bool reloadAResourceGroup(const std::string& pResourceGroupName,const std::vector<ArchivePathAndType>& pLocationToAdd);
   
   /// \brief this serves the same purpose than reloadAResourceGroup, but it does not destroy/recreate the ResourceGroup.
   /// \param the name of the resourcegroup
   /// on the one hand, you don't need to worry about the path and order of the locations.
   /// on the other hand, there is no test that the resources were really cleared and reloaded.
   /// personnally I prefer this thought, because it's much smarter & less costly in the end.
   bool reloadAResourceGroupWithoutDestroyingIt(const std::string& pResourceGroupName);

   /// \brief return true if the resourcegroup exists
   /// \param the name of the resourcegroup
   bool resourceGroupExist(const std::string& pResourceGroupName);

   /// \brief updating informations about materials on all 'reachable' renderables
   /// that are currently used by the different scenemanagers and overlays
   void updateOnEveryRenderable();

   /// \brief get the latest modification time : check all files date from the resourcegroup
   /// \param the name of the resourcegroup
   /// \warning time-costly! because access the HDD!
   time_t getLatestModificationTime(const std::string& pResourceGroupName);


   /// \brief test latest modification time.
   /// if it did change since the latest call to this function,
   /// then the resourcegroup is reloaded
   /// and all the renderables are updated
   /// bool returns if reload was tried or not.
   /// if an error happens during reloading (parsing script + glsl), it is likely to be
   ///      described in the pOutLoggedMessages param, if useLog.
   bool checkTimeAndReloadIfNeeded(const std::string& pResourceGroupName,std::string &pOutLoggedMessages, bool useLog=true);
};

#endif