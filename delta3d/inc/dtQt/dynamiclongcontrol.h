/* -*-c++-*-
 * Delta3D Simulation Training And Game Editor (STAGE)
 * STAGE - dynamiclongcontrol (.h & .cpp) - Using 'The MIT License'
 * Copyright (C) 2005-2008, Alion Science and Technology Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * This software was developed by Alion Science and Technology Corporation under
 * circumstances in which the U. S. Government may have rights in the software.
 *
 * Curtiss Murphy
 */
#ifndef DELTA_DYNAMICLONGCONTROL
#define DELTA_DYNAMICLONGCONTROL

#include <dtQt/export.h>
#include <dtQt/dynamicabstractcontrol.h>
#include <dtQt/dynamicsubwidgets.h>

class QWidget;

namespace dtCore
{
    class LongActorProperty;
}

namespace dtQt
{

    /**
    * @class DynamicLongControl
    * @brief This is the dynamic control for the long data type - used in the property editor.
    * @note This is really the same as int, but we need a separate control anyway.
    */
    class DT_QT_EXPORT DynamicLongControl : public DynamicAbstractControl
    {
        Q_OBJECT

    public:
        /**
         * Constructor
         */
        DynamicLongControl();

        /**
         * Destructor
         */
        virtual ~DynamicLongControl();

        /**
         * @see DynamicAbstractControl#InitializeData
         */
        virtual void InitializeData(DynamicAbstractControl* newParent, PropertyEditorModel* model,
            dtCore::PropertyContainer* newPC, dtCore::ActorProperty* property);

        /**
         * @see DynamicAbstractControl#updateEditorFromModel
         */
        virtual void updateEditorFromModel(QWidget* widget);

        /**
         * @see DynamicAbstractControl#updateModelFromEditor
         */
        virtual bool updateModelFromEditor(QWidget* widget);

        /**
         * @see DynamicAbstractControl#createEditor
         */
        virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
            const QModelIndex& index);

        /**
         * @see DynamicAbstractControl#getDisplayName
         */
        virtual const QString getDisplayName();

         /**
         * @see DynamicAbstractControl#getDescription
         */
        virtual const QString getDescription();

         /**
         * @see DynamicAbstractControl#getValueAsString
         */
        virtual const QString getValueAsString();

   public slots:

        /**
         * @see DynamicAbstractControl#updateData
         */
        virtual bool updateData(QWidget* widget);

        /**
         * @see DynamicAbstractControl#handleSubEditDestroy
         */
        void handleSubEditDestroy(QWidget* widget, QAbstractItemDelegate::EndEditHint hint = QAbstractItemDelegate::NoHint);

    protected:

    private:
        dtCore::LongActorProperty* mProperty;

        // This pointer is not really in our control.  It is constructed in the createEditor()
        // method and destroyed whenever QT feels like it (mostly when the control looses focus).
        // We work around this by trapping the destruction of this object, it should
        // call our handleSubEditDestroy() method so we know to not hold this anymore
        SubQLineEdit* mTemporaryEditControl;
    };

} // namespace dtQt

#endif // DELTA_DYNAMICLONGCONTROL
