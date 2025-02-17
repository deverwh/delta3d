#ifndef DELTA_MainWindow
#define DELTA_MainWindow

#include <QtGui/QMainWindow>

class QAction;
class QTableWidgetItem;
class QToolBar;
class AnimationTableWidget;
class QLabel;
class QListWidget;
class QListWidgetItem;
class QTableWidget;
class QGraphicsView;
class QGraphicsScene;
class QTabWidget;
class QGridLayout;
class QStandardItemModel;
class QTableView;
class PoseMeshView;
class PoseMeshScene;
class PoseMeshProperties;
class QDoubleSpinBox;
class QComboBox;
class QHBoxLayout;
class QTreeWidget;
class QTreeWidgetItem;
class QPushButton;

namespace dtAnim
{
   class PoseMesh;
   class Cal3DModelData;
   class Cal3DModelWrapper;
   class CharDrawable;
}

// Application Level Classes
class Viewer; ///<The animation viewer application
class FileItemDelegate;
class ObjectNameItemDelegate;

class MainWindow : public QMainWindow
{
   friend class Delta3DThread;
   Q_OBJECT
public:
   MainWindow();
   ~MainWindow();

   void LoadCharFile(const QString& filename);
   void SaveCharFile(const QString& filename);
   void ReloadCharFile();
   void SetViewer(Viewer* viewer);

   void LoadAttachment(const QString& filename);

protected:
   virtual void dragEnterEvent(QDragEnterEvent* event);
   virtual void dropEvent(QDropEvent* event);

signals:
   void NewFile();
   void ReloadFile();
   void FileToLoad(const QString&);
   void FileToSave(const QString&);
   void UnloadFile();
   void ClearTempFile();
   void StartAnimation(unsigned int, float, float);
   void StopAnimation(unsigned int, float);
   void StartAction(unsigned int, float, float);
   void LODScale_Changed(float scaleValue);
   void SpeedChanged(float speedFactor);
   void ScaleFactorChanged(float scaleFactorValue);
   void SetBoneLabeling(bool shouldShow);

   //Show the mesh on the CalModel
   void ShowMesh(int meshID);
   /// Hide the mesh on CalModel from view
   void HideMesh(int meshID);

   void SubMorphTargetChanged(int meshID, int subMeshID,
                              int morphID, float weight);

   void PlayMorphAnimation(int morphAnimID, float weight, float delayIn, float delayOut, bool looping);
   void StopMorphAnimation(int morphAnimID, float delay);

   void AttachmentToLoad(const QString&);

   void AttachmentSettingsChanged(const std::string& boneName, float offsetX, float offsetY, float offsetZ, float rotx, float roty, float rotz);

public slots:
   void OnNewAnimation(unsigned int id, const QString& animationName, unsigned int trackCount,
                       unsigned int keyframes, float duration);

   void OnNewMorphAnimation(unsigned int id, const QString& animationName, unsigned int trackCount,
      unsigned int keyframes, float duration);

   void OnNewMesh(int meshID, const QString& meshName, const std::vector<std::string>& boneNames);

   void OnNewSubMorphTarget(int meshID, int subMeshID, 
                            int morphID, const QString& morphName);

   void OnPoseMeshesLoaded(const std::vector<dtAnim::PoseMesh*>& poseMeshList,
                           dtAnim::CharDrawable* model);

   void OnNewMaterial(int matID, const QString& name,
                      const QColor& diff, const QColor& amb, const QColor& spec,
                      float shininess);

   void OnCharacterDataLoaded(dtAnim::Cal3DModelData* modelData, dtAnim::Cal3DModelWrapper* wrapper);

   void OnBlendUpdate(const std::vector<std::pair<float, float> >& animWeightTimeList, const std::vector<float>& morphWeightList);

   void OnAnimationClicked(QTableWidgetItem* item);
   void OnMorphAnimationClicked(QTableWidgetItem* item);
   void OnMeshActivated(QListWidgetItem* item);
   void OnLODScale_Changed(double newValue);
   void OnSpeedChanged(double newValue);
   void OnChangeScaleFactor();
   void OnToggleHardwareSkinning();
   void OnToggleShadingToolbar();
   void OnToggleLightingToolbar();
   void OnDisplayError(const QString& msg);

   void OnClearCharacterData();

   void OnLoadAttachment();
   void OnChangeAttachmentSettings();

   void OnResourceEditStart(int fileType, const std::string& objectName);
   void OnResourceEditEnd(int fileType, const std::string& objectName);
   void OnResourceNameChanged(int fileType, const std::string& oldName, const std::string& newName) const;
   void OnResourceAdd();
   void OnResourceRemoved(int fileType, const std::string& objectName);

   void OnError(const std::string& title, const std::string& message);
   
   void OnInitialization();

private:
   void CreateMenus();
   void CreateActions();
   void CreateToolbars();
   void CreateDockWidgets();
   void CreateDockWidget_Properties();
   void CreateDockWidget_Tools();
   void CreateDockWidget_Resources();
   void DestroyPoseResources();
   void UpdateRecentFileActions();
   void SetCurrentFile(const QString& filename);
   void OnStartAnimation(int row);
   void OnStopAnimation(int row);
   void OnStartMorphAnimation(int row, bool looping);
   void OnStopMorphAnimation(int row, float delay);
   void OnStartAction(int row);
   bool IsAnimNodeBuildingUsingHW() const;
   void SetupConnectionsWithViewer();

   ///turns color into "R:rrr "G:ggg B:bbb A:aaa" format
   QString MakeColorString(const QColor& color) const;

   void SaveSettings();
   bool IsShaderDefFileValid() const;
   bool EnsureShaderDefFileValid();
   bool AskUserToLoadShaderDef();


   std::string mContextPath;
   std::string mShaderDefFile;

   QAction* mExitAct;
   QAction* mNewCharAct;
   QAction* mLoadCharAct;
   QAction* mSaveCharAct;
   QAction* mCloseCharAction;
   QAction* mRecentFilesAct[5];
   QAction* mWireframeAction;
   QAction* mShadedAction;
   QAction* mShadedWireAction;
   QAction* mBoneBasisAction;
   QAction* mDiffuseLightAction;
   QAction* mPointLightAction;
   QAction* mHardwareSkinningAction;
   QAction* mBoneLabelAction;
   QAction* mToggleDockProperties;
   QAction* mToggleDockResources;
   QAction* mToggleDockTools;

   QToolBar* mShadingToolbar;
   QToolBar* mLightingToolbar;

   QDoubleSpinBox* mScaleFactorSpinner;

   QDoubleSpinBox* mAttachmentOffsetXSpinner;
   QDoubleSpinBox* mAttachmentOffsetYSpinner;
   QDoubleSpinBox* mAttachmentOffsetZSpinner;
   QDoubleSpinBox* mAttachmentRotXSpinner;
   QDoubleSpinBox* mAttachmentRotYSpinner;
   QDoubleSpinBox* mAttachmentRotZSpinner;
   QComboBox* mAttachmentParent;
   QString mCurrentAttachment;   // the currently loaded attachment mesh
   
   QTabWidget* mTabs;

   AnimationTableWidget* mAnimListWidget;
   QListWidget*          mMeshListWidget;
   QTableWidget*         mSubMorphTargetListWidget;
   AnimationTableWidget* mSubMorphAnimationListWidget;

   QStandardItemModel* mMaterialModel; ///<Model for the character's materials
   QTableView*         mMaterialView;  ///<View for the character's materials

   QDockWidget*        mPoseDock;
   PoseMeshView*       mPoseMeshViewer;
   PoseMeshScene*      mPoseMeshScene;
   PoseMeshProperties* mPoseMeshProperties;
   
   QLabel*             mFileLabel;
   QPushButton*        mFileAdd;
   QTreeWidget*        mFileTree;
   QTreeWidgetItem*    mFileGroupSkel;
   QTreeWidgetItem*    mFileGroupAnim;
   QTreeWidgetItem*    mFileGroupMesh;
   QTreeWidgetItem*    mFileGroupMat;
   QTreeWidgetItem*    mFileGroupMorph;

   Viewer*             mViewer;
   QHBoxLayout*        mCentralLayout;

   FileItemDelegate*   mFileDelegate;
   ObjectNameItemDelegate* mObjectNameDelegate;

   std::string mCurrentFile;

private slots:
   void OnNewCharFile();
   void OnOpenCharFile();
   void OpenRecentFile();

   void OnSaveCharFile();
      
   void OnCloseCharFile();

   void OnItemChanged(QTableWidgetItem* item);
   void OnItemDoubleClicked(QTableWidgetItem* item);

   void OnMorphItemChanged(QTableWidgetItem* item);
   void OnMorphItemDoubleClicked(QTableWidgetItem* item);

   void OnSelectModeGrab();
   void OnSelectModeBlendPick();
   void OnSelectModeErrorPick();
   void OnSelectLookAtCamera();

   void OnToggleDisplayEdges(bool shouldDisplay);
   void OnToggleDisplayError(bool shouldDisplay);
   void OnToggleFlipVertical();
   void OnToggleFlipHorizontal();

   void OnSubMorphChanged(QTableWidgetItem* item);

   void OnLoadShaderDefinition();
   void OnLoadShaderFile(const QString& filename);
   void OnReloadShaderFiles();
};
#endif // DELTA_MainWindow
