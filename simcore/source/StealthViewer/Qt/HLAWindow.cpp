/* -*-c++-*-
* Stealth Viewer
* Copyright 2007-2008, Alion Science and Technology
*
* This library is free software; you can redistribute it and/or modify it under
* the terms of the GNU Lesser General Public License as published by the Free
* Software Foundation; either version 2.1 of the License, or (at your option)
* any later version.
*
* This library is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
* FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
* details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this library; if not, write to the Free Software Foundation, Inc.,
* 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*
* This software was developed by Alion Science and Technology Corporation under
* circumstances in which the U. S. Government may have rights in the software.
 * @author Eddie Johnson
 */
#include <prefix/StealthQtPrefix.h>
#include <StealthViewer/Qt/HLAWindow.h>
#include <ui_HLAWindowUi.h>
#include <StealthViewer/Qt/HLAOptions.h>
#include <StealthViewer/Qt/StealthViewerSettings.h>
#include <StealthViewer/Qt/StealthViewerData.h>
#include <StealthViewer/GMApp/PreferencesGeneralConfigObject.h>
#include <QtGui/QMessageBox>
#include <dtHLAGM/hlacomponent.h>
#include <dtHLAGM/hlacomponentconfig.h>
#include <dtCore/project.h>
#include <dtCore/resourcedescriptor.h>
#include <dtUtil/fileutils.h>
#include <dtUtil/macros.h>
#include <SimCore/HLA/HLAConnectionComponent.h>

#ifdef DELTA_WIN32
   bool caseSensitive = false;
#else
   bool caseSensitive = true;
#endif

namespace StealthQt
{
   HLAWindow::HLAWindow(dtGame::GameManager &gm,
                        QWidget *parent,
                        StealthViewerSettings *settings,
                        bool isConnected,
                        QString currentConnectionName) :
      QDialog(parent),
      mUi(new Ui::HLAWindow),
      mIsConnected(isConnected),
      mHLAComp(NULL),
      mCurrentConnectionName(currentConnectionName),
      mCancelConnectProcess(false)
   {
      mUi->setupUi(this);

      ConnectSlots();

      // Display saved connections
      QStringList toDisplay;
      toDisplay = (settings != NULL) ? settings->GetConnectionNames()
                : StealthViewerData::GetInstance().GetSettings().GetConnectionNames();

      mUi->mNetworkListWidget->addItems(toDisplay);

      mHLAComp =
         static_cast<SimCore::HLA::HLAConnectionComponent*>
         (gm.GetComponentByName(SimCore::HLA::HLAConnectionComponent::DEFAULT_NAME));

      mUi->mConnectPushButton->setEnabled(!mIsConnected && mUi->mNetworkListWidget->currentItem() != NULL);
      mUi->mDisconnectPushButton->setEnabled(mIsConnected);
      mUi->mEditPushButton->setEnabled(false);
      mUi->mDeletePushButton->setEnabled(false);

      if(!mCurrentConnectionName.isEmpty())
         mUi->mCurrentConnectionLineEdit->setText(mCurrentConnectionName);

      bool connect =
         StealthViewerData::GetInstance().GetGeneralConfigObject().GetReconnectOnStartup();

      mUi->mReconnectOnStartupCheckBox->setChecked(connect);

      //automatically select the first item in the network list, if there is one
      if (mUi->mNetworkListWidget->item(0))
      {
         mUi->mNetworkListWidget->setCurrentItem(mUi->mNetworkListWidget->item(0));
      }
   }

   //////////////////////////////////////////////////////////////
   HLAWindow::~HLAWindow()
   {
      delete mUi;
      mUi = NULL;
   }

   //////////////////////////////////////////////////////////////
   QListWidget* HLAWindow::GetNetworkListWidget()
   {
      return mUi->mNetworkListWidget;
   }

   //////////////////////////////////////////////////////////////
   void HLAWindow::ConnectSlots()
   {
      connect(mUi->mConnectPushButton,    SIGNAL(clicked(bool)), this, SLOT(OnConnect(bool)));
      connect(mUi->mDisconnectPushButton, SIGNAL(clicked(bool)), this, SLOT(OnDisconnect(bool)));
      connect(mUi->mClosePushButton,      SIGNAL(clicked(bool)), this, SLOT(OnClose(bool)));
      connect(mUi->mNewPushButton,        SIGNAL(clicked(bool)), this, SLOT(OnNew(bool)));
      connect(mUi->mEditPushButton,       SIGNAL(clicked(bool)), this, SLOT(OnEdit(bool)));
      connect(mUi->mDeletePushButton,     SIGNAL(clicked(bool)), this, SLOT(OnDelete(bool)));

      connect(mUi->mNetworkListWidget,
              SIGNAL(currentTextChanged(const QString&)),
              this,
              SLOT(OnCurrentTextChanged(const QString&)));

      connect(&StealthViewerData::GetInstance().GetSettings(),
               SIGNAL(ItemDeleted(QString)),
               this,
               SLOT(OnListItemDeleted(QString)));
   }

   //////////////////////////////////////////////////////////////
   void HLAWindow::OnConnect(bool checked)
   {
      if( mHLAComp == NULL )
      {
         QMessageBox::warning(this, tr("Error"),
            tr("Cannot connect to network because network component was not initialized."), QMessageBox::Ok);
         return;
      }

      if( mHLAComp->GetConnectionState() !=
         SimCore::HLA::HLAConnectionComponent::ConnectionState::STATE_NOT_CONNECTED )
      {
         OnDisconnect();

         // If the disconnect from current network message box
         // is picked to cancel, this flag will be set to true
         // in which case we need to leave this method.

         // This is done since OnDisconnect is a SLOT, it's return value
         // CANNOT be changed to mismatch the SIGNAL prototype.
         if(mCancelConnectProcess)
         {
            // Make sure to flip this back to false
            mCancelConnectProcess = false;
            return;
         }
      }

      QListWidgetItem *currentItem = mUi->mNetworkListWidget->currentItem();
      if(currentItem == NULL)
      {
         QMessageBox::warning(this, tr("Error"),
            tr("Please select a network connection from the list"), QMessageBox::Ok);
         return;
      }

      mUi->mCurrentConnectionLineEdit->setText(currentItem->text());
      mCurrentConnectionName = currentItem->text();

      QString fedEx = currentItem->text();

      QStringList props =
         StealthViewerData::GetInstance().GetSettings().GetConnectionProperties(fedEx);

      SetConnectionValues(props);

      mIsConnected = true;

      mUi->mDisconnectPushButton->setEnabled( mIsConnected );
      mUi->mConnectPushButton->setEnabled( ! mIsConnected );

      // connect to network
      OnClose();

      accept();
   }

   //////////////////////////////////////////////////////////////
   void HLAWindow::OnDisconnect(bool checked)
   {
      // Check to see if we are connected first

      int result = QMessageBox::question(this, tr("Confirm Disconnection"),
         tr("Are you sure you want to disconnect?"), QMessageBox::Yes, QMessageBox::No);

      if(result == QMessageBox::Yes)
      {
         // Disconnect from network
         if(mHLAComp != NULL)
         {
            mHLAComp->Disconnect();
         }

         // Disable applicable buttons
         mUi->mCurrentConnectionLineEdit->setText("None");
         mCurrentConnectionName = mUi->mCurrentConnectionLineEdit->text();
         mUi->mDisconnectPushButton->setEnabled(false);
         mUi->mConnectPushButton->setEnabled(true);

         if(mUi->mNetworkListWidget->currentItem() != NULL)
         {
            mUi->mEditPushButton->setEnabled(true);
            mUi->mDeletePushButton->setEnabled(true);
         }

         mIsConnected = false;

         emit DisconnectedFromNetwork();
      }
      else
      {
         // Cancel, because they selected QMessageBox::No
         mCancelConnectProcess = true;
      }
   }

   //////////////////////////////////////////////////////////////
   void HLAWindow::OnClose(bool checked)
   {
      // Write the reconnect property to the settings file
      // This way it is updated whenever a new HLAWindow was closed
      StealthGM::PreferencesGeneralConfigObject &genConfig =
         StealthViewerData::GetInstance().GetGeneralConfigObject();

      genConfig.SetReconnectOnStartup(mUi->mReconnectOnStartupCheckBox->isChecked(),
         mCurrentConnectionName != "" ? mCurrentConnectionName.toStdString() : "");

      close();
   }

   //////////////////////////////////////////////////////////////
   void HLAWindow::OnNew(bool checked)
   {
      HLAOptions options(this);
      options.SetCaseSensitiveFilePaths(caseSensitive);
      if(options.exec() == QDialog::Accepted)
      {
         QString name = options.GetConnectionName();
         if(!name.isEmpty())
            mUi->mNetworkListWidget->addItem(name);
      }
   }

   //////////////////////////////////////////////////////////////
   void HLAWindow::OnEdit(bool checked)
   {
      QListWidgetItem *currentItem = mUi->mNetworkListWidget->currentItem();
      if(currentItem == NULL)
      {
         QMessageBox::information(this, tr("Error"),
            tr("Please select a network connection to edit."), QMessageBox::Ok);

         return;
      }

      HLAOptions options(this, currentItem->text(), true);
      options.SetCaseSensitiveFilePaths(caseSensitive);
      if(options.exec() == QDialog::Accepted)
      {
         QStringList toDisplay =
            StealthViewerData::GetInstance().GetSettings().GetConnectionNames();

         mUi->mNetworkListWidget->clear();
         mUi->mNetworkListWidget->addItems(toDisplay);
      }
   }

   //////////////////////////////////////////////////////////////
   void HLAWindow::OnDelete(bool checked)
   {
      int result =
         QMessageBox::information(this, tr("Confirm Delete"),
            tr("Are you sure you want to delete this connection?"),
            QMessageBox::Yes, QMessageBox::No);

      if(result == QMessageBox::No)
         return;

      QListWidgetItem *currentItem = mUi->mNetworkListWidget->currentItem();
      if(currentItem == NULL)
      {
         QMessageBox::information(this, tr("Error"),
            tr("Please select a network connection to delete."), QMessageBox::Ok);

         return;
      }

      StealthViewerSettings &settings = StealthViewerData::GetInstance().GetSettings();
      settings.RemoveConnection(currentItem->text());
   }

   //////////////////////////////////////////////////////////////
   void HLAWindow::OnListItemDeleted(QString name)
   {
      mUi->mNetworkListWidget->clear();
      StealthViewerSettings &settings = StealthViewerData::GetInstance().GetSettings();
      mUi->mNetworkListWidget->addItems(settings.GetConnectionNames());
   }

   //////////////////////////////////////////////////////////////
   void HLAWindow::SetConnectionValues(QStringList& properties)
   {
      try
      {
         dtCore::Project& project = dtCore::Project::GetInstance();

         SimCore::HLA::DISConnectionData disConnectionData;

         std::string connectionType = properties[8].toStdString();

         if(mHLAComp != NULL)
         {
            std::string map = properties[1].toStdString();
            if (!map.empty())
            {
               // Assign the primary map to load
               mHLAComp->AddMap(map);
            }

            mHLAComp->SetFedEx(properties[4].toStdString());
            mHLAComp->SetFedName(properties[5].toStdString());

            mHLAComp->SetRTIStandard(properties[7].toStdString());

            mHLAComp->SetServerIPAddress(properties[9].toStdString());
            mHLAComp->SetServerPort(properties[10].toStdString());
            mHLAComp->SetServerGameName(properties[11].toStdString());
            mHLAComp->SetServerGameVersion(properties[12].toInt());

            disConnectionData.SetIPAddress(properties[13].toStdString());
            disConnectionData.SetPort(properties[14].toUInt());
            disConnectionData.SetBroadcastPort(dtUtil::ToType<bool>(properties[15].toStdString()));
            disConnectionData.SetExerciseId(properties[16].toUInt());
            disConnectionData.SetSiteId(properties[17].toUShort());
            disConnectionData.SetApplicationId(properties[18].toUShort());
            disConnectionData.SetMTU(properties[19].toUInt());
            disConnectionData.SetActorXMLFile(properties[20].toStdString());

            mHLAComp->SetDISConnectionData(disConnectionData);

            if (connectionType == StealthViewerSettings::CONNECTIONTYPE_HLA)
            {
               // only read the connection files if this is for HLA.  This could be fixed if the connection component
               // just took resource descriptors like it should.
               mHLAComp->SetConfigFile(project.GetResourcePath(dtCore::ResourceDescriptor(properties[2].toStdString())));
               mHLAComp->SetFedFile(project.GetResourcePath(dtCore::ResourceDescriptor(properties[3].toStdString())));

               // The rid file is not required, and many newer rtis don't even have such a concept.
               if (!properties[6].isEmpty())
               {
                  mHLAComp->SetRidFile(project.GetResourcePath(dtCore::ResourceDescriptor(properties[6].toStdString())));
               }
               else
               {
                  mHLAComp->SetRidFile("");
               }

               mHLAComp->SetConnectionType(SimCore::HLA::HLAConnectionComponent::ConnectionType::TYPE_HLA);
            }
            else if (connectionType == StealthViewerSettings::CONNECTIONTYPE_CLIENTSERVER)
            {
               mHLAComp->SetConnectionType(SimCore::HLA::HLAConnectionComponent::ConnectionType::TYPE_CLIENTSERVER);
            }
            else if (connectionType == StealthViewerSettings::CONNECTIONTYPE_DIS)
            {
               mHLAComp->SetConnectionType(SimCore::HLA::HLAConnectionComponent::ConnectionType::TYPE_DIS);
            }
            else 
            {
               mHLAComp->SetConnectionType(SimCore::HLA::HLAConnectionComponent::ConnectionType::TYPE_NONE);
            }

            try
            {
               mHLAComp->StartNetworkConnection();
            }
            catch(const dtUtil::Exception &e)
            {
               QMessageBox::warning(this, tr("Error"),
                  tr("An error occurred while trying to connect to the network: ") +
                  tr(e.What().c_str()), QMessageBox::Ok);
            }

            emit ConnectedToNetwork(properties[0]);
         }
      }
      catch(const dtUtil::Exception &ex)
      {
         QMessageBox::warning(this, tr("Error"),
               tr("Error searching for network resource files. Unable to connect to the network: ") + tr(ex.ToString().c_str()),
               QMessageBox::Ok);
         return;
      }
   }

   //////////////////////////////////////////////////////////////
   void HLAWindow::OnCurrentTextChanged(const QString &str)
   {
      mUi->mConnectPushButton->setEnabled(!str.isEmpty() && !mIsConnected);
      //mUi->mDisconnectPushButton->setEnabled(mIsConnected);
      mUi->mEditPushButton->setEnabled(!str.isEmpty() && !mIsConnected);
      mUi->mDeletePushButton->setEnabled(!str.isEmpty() && !mIsConnected);
   }

   //////////////////////////////////////////////////////////////
   void HLAWindow::OnListItemActivated(QListWidgetItem *item)
   {
      if(item != NULL)
      {
         QString str = item->text();

         mUi->mEditPushButton->setEnabled(!str.isEmpty() && !mIsConnected);
         mUi->mDeletePushButton->setEnabled(!str.isEmpty() && !mIsConnected);
      }
   }
}
