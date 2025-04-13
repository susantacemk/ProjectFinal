/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.7.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTableView>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout_6;
    QWidget *widget;
    QVBoxLayout *verticalLayout;
    QVBoxLayout *verticalLayout_5;
    QWidget *widget_2;
    QHBoxLayout *horizontalLayout;
    QComboBox *comboBox;
    QSpacerItem *horizontalSpacer;
    QComboBox *comboBox_2;
    QSpacerItem *horizontalSpacer_2;
    QPushButton *pushButton;
    QSpacerItem *horizontalSpacer_3;
    QPushButton *pushButton_2;
    QSpacerItem *horizontalSpacer_4;
    QPushButton *pushButton_3;
    QSplitter *splitter_2;
    QWidget *widget_3;
    QVBoxLayout *verticalLayout_2;
    QWidget *widget_4;
    QHBoxLayout *horizontalLayout_2;
    QComboBox *comboBox_3;
    QPushButton *pushButton_5;
    QPushButton *pushButton_4;
    QTreeView *treeView;
    QWidget *widget_5;
    QVBoxLayout *verticalLayout_4;
    QSplitter *splitter;
    QTabWidget *tabWidget;
    QWidget *tab;
    QWidget *tab_2;
    QWidget *widget_6;
    QVBoxLayout *verticalLayout_3;
    QWidget *widget_7;
    QHBoxLayout *horizontalLayout_3;
    QPushButton *pushButton_6;
    QPushButton *pushButton_7;
    QPushButton *pushButton_8;
    QPushButton *pushButton_9;
    QPushButton *pushButton_10;
    QPushButton *pushButton_11;
    QTabWidget *tabWidget_2;
    QWidget *tab_3;
    QWidget *tab_4;
    QTableView *tableView;
    QMenuBar *menuBar;
    QMenu *menuFile;
    QMenu *menuEdit;
    QMenu *menuView;
    QMenu *menuRefractor;
    QMenu *menuRun;
    QMenu *menuProjects;
    QMenu *menuWindow;
    QToolBar *toolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(1112, 609);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        verticalLayout_6 = new QVBoxLayout(centralwidget);
        verticalLayout_6->setObjectName("verticalLayout_6");
        widget = new QWidget(centralwidget);
        widget->setObjectName("widget");
        verticalLayout = new QVBoxLayout(widget);
        verticalLayout->setObjectName("verticalLayout");
        verticalLayout_5 = new QVBoxLayout();
        verticalLayout_5->setObjectName("verticalLayout_5");
        widget_2 = new QWidget(widget);
        widget_2->setObjectName("widget_2");
        widget_2->setMaximumSize(QSize(16777215, 40));
        widget_2->setStyleSheet(QString::fromUtf8("border: 1px solid green"));
        horizontalLayout = new QHBoxLayout(widget_2);
        horizontalLayout->setObjectName("horizontalLayout");
        comboBox = new QComboBox(widget_2);
        comboBox->setObjectName("comboBox");
        QSizePolicy sizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(comboBox->sizePolicy().hasHeightForWidth());
        comboBox->setSizePolicy(sizePolicy);
        comboBox->setMaximumSize(QSize(16777215, 40));
        comboBox->setMouseTracking(false);

        horizontalLayout->addWidget(comboBox);

        horizontalSpacer = new QSpacerItem(921, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        comboBox_2 = new QComboBox(widget_2);
        comboBox_2->setObjectName("comboBox_2");
        sizePolicy.setHeightForWidth(comboBox_2->sizePolicy().hasHeightForWidth());
        comboBox_2->setSizePolicy(sizePolicy);

        horizontalLayout->addWidget(comboBox_2);

        horizontalSpacer_2 = new QSpacerItem(207, 15, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_2);

        pushButton = new QPushButton(widget_2);
        pushButton->setObjectName("pushButton");

        horizontalLayout->addWidget(pushButton);

        horizontalSpacer_3 = new QSpacerItem(206, 15, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_3);

        pushButton_2 = new QPushButton(widget_2);
        pushButton_2->setObjectName("pushButton_2");

        horizontalLayout->addWidget(pushButton_2);

        horizontalSpacer_4 = new QSpacerItem(207, 15, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_4);

        pushButton_3 = new QPushButton(widget_2);
        pushButton_3->setObjectName("pushButton_3");

        horizontalLayout->addWidget(pushButton_3);


        verticalLayout_5->addWidget(widget_2);

        splitter_2 = new QSplitter(widget);
        splitter_2->setObjectName("splitter_2");
        splitter_2->setOrientation(Qt::Orientation::Horizontal);
        widget_3 = new QWidget(splitter_2);
        widget_3->setObjectName("widget_3");
        widget_3->setStyleSheet(QString::fromUtf8("border: 1px solid green"));
        verticalLayout_2 = new QVBoxLayout(widget_3);
        verticalLayout_2->setObjectName("verticalLayout_2");
        widget_4 = new QWidget(widget_3);
        widget_4->setObjectName("widget_4");
        horizontalLayout_2 = new QHBoxLayout(widget_4);
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        comboBox_3 = new QComboBox(widget_4);
        comboBox_3->setObjectName("comboBox_3");

        horizontalLayout_2->addWidget(comboBox_3);

        pushButton_5 = new QPushButton(widget_4);
        pushButton_5->setObjectName("pushButton_5");

        horizontalLayout_2->addWidget(pushButton_5);

        pushButton_4 = new QPushButton(widget_4);
        pushButton_4->setObjectName("pushButton_4");

        horizontalLayout_2->addWidget(pushButton_4);


        verticalLayout_2->addWidget(widget_4);

        treeView = new QTreeView(widget_3);
        treeView->setObjectName("treeView");

        verticalLayout_2->addWidget(treeView);

        splitter_2->addWidget(widget_3);
        widget_5 = new QWidget(splitter_2);
        widget_5->setObjectName("widget_5");
        widget_5->setStyleSheet(QString::fromUtf8("border: 1px solid green"));
        verticalLayout_4 = new QVBoxLayout(widget_5);
        verticalLayout_4->setObjectName("verticalLayout_4");
        splitter = new QSplitter(widget_5);
        splitter->setObjectName("splitter");
        splitter->setOrientation(Qt::Orientation::Vertical);
        tabWidget = new QTabWidget(splitter);
        tabWidget->setObjectName("tabWidget");
        tabWidget->setStyleSheet(QString::fromUtf8("border: 1px solid black"));
        tabWidget->setTabShape(QTabWidget::TabShape::Triangular);
        tabWidget->setDocumentMode(true);
        tabWidget->setTabsClosable(true);
        tabWidget->setMovable(true);
        tabWidget->setTabBarAutoHide(true);
        tab = new QWidget();
        tab->setObjectName("tab");
        tabWidget->addTab(tab, QString());
        tab_2 = new QWidget();
        tab_2->setObjectName("tab_2");
        tabWidget->addTab(tab_2, QString());
        splitter->addWidget(tabWidget);
        widget_6 = new QWidget(splitter);
        widget_6->setObjectName("widget_6");
        verticalLayout_3 = new QVBoxLayout(widget_6);
        verticalLayout_3->setObjectName("verticalLayout_3");
        widget_7 = new QWidget(widget_6);
        widget_7->setObjectName("widget_7");
        widget_7->setStyleSheet(QString::fromUtf8("border: 1px solid black"));
        horizontalLayout_3 = new QHBoxLayout(widget_7);
        horizontalLayout_3->setObjectName("horizontalLayout_3");
        pushButton_6 = new QPushButton(widget_7);
        pushButton_6->setObjectName("pushButton_6");

        horizontalLayout_3->addWidget(pushButton_6);

        pushButton_7 = new QPushButton(widget_7);
        pushButton_7->setObjectName("pushButton_7");

        horizontalLayout_3->addWidget(pushButton_7);

        pushButton_8 = new QPushButton(widget_7);
        pushButton_8->setObjectName("pushButton_8");

        horizontalLayout_3->addWidget(pushButton_8);

        pushButton_9 = new QPushButton(widget_7);
        pushButton_9->setObjectName("pushButton_9");

        horizontalLayout_3->addWidget(pushButton_9);

        pushButton_10 = new QPushButton(widget_7);
        pushButton_10->setObjectName("pushButton_10");

        horizontalLayout_3->addWidget(pushButton_10);

        pushButton_11 = new QPushButton(widget_7);
        pushButton_11->setObjectName("pushButton_11");

        horizontalLayout_3->addWidget(pushButton_11);


        verticalLayout_3->addWidget(widget_7);

        tabWidget_2 = new QTabWidget(widget_6);
        tabWidget_2->setObjectName("tabWidget_2");
        tabWidget_2->setStyleSheet(QString::fromUtf8(""));
        tabWidget_2->setTabShape(QTabWidget::TabShape::Triangular);
        tabWidget_2->setDocumentMode(true);
        tabWidget_2->setTabsClosable(true);
        tabWidget_2->setMovable(true);
        tabWidget_2->setTabBarAutoHide(true);
        tab_3 = new QWidget();
        tab_3->setObjectName("tab_3");
        tabWidget_2->addTab(tab_3, QString());
        tab_4 = new QWidget();
        tab_4->setObjectName("tab_4");
        tabWidget_2->addTab(tab_4, QString());

        verticalLayout_3->addWidget(tabWidget_2);

        splitter->addWidget(widget_6);

        verticalLayout_4->addWidget(splitter);

        splitter_2->addWidget(widget_5);
        tableView = new QTableView(splitter_2);
        tableView->setObjectName("tableView");
        tableView->setStyleSheet(QString::fromUtf8("border: 1px solid blue"));
        splitter_2->addWidget(tableView);

        verticalLayout_5->addWidget(splitter_2);


        verticalLayout->addLayout(verticalLayout_5);


        verticalLayout_6->addWidget(widget);

        MainWindow->setCentralWidget(centralwidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName("menuBar");
        menuBar->setGeometry(QRect(0, 0, 1112, 26));
        menuFile = new QMenu(menuBar);
        menuFile->setObjectName("menuFile");
        menuEdit = new QMenu(menuBar);
        menuEdit->setObjectName("menuEdit");
        menuView = new QMenu(menuBar);
        menuView->setObjectName("menuView");
        menuRefractor = new QMenu(menuBar);
        menuRefractor->setObjectName("menuRefractor");
        menuRun = new QMenu(menuBar);
        menuRun->setObjectName("menuRun");
        menuProjects = new QMenu(menuBar);
        menuProjects->setObjectName("menuProjects");
        menuWindow = new QMenu(menuBar);
        menuWindow->setObjectName("menuWindow");
        MainWindow->setMenuBar(menuBar);
        toolBar = new QToolBar(MainWindow);
        toolBar->setObjectName("toolBar");
        MainWindow->addToolBar(Qt::ToolBarArea::LeftToolBarArea, toolBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName("statusBar");
        MainWindow->setStatusBar(statusBar);

        menuBar->addAction(menuFile->menuAction());
        menuBar->addAction(menuEdit->menuAction());
        menuBar->addAction(menuView->menuAction());
        menuBar->addAction(menuRefractor->menuAction());
        menuBar->addAction(menuRun->menuAction());
        menuBar->addAction(menuProjects->menuAction());
        menuBar->addAction(menuWindow->menuAction());

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        pushButton->setText(QCoreApplication::translate("MainWindow", "PushButton", nullptr));
        pushButton_2->setText(QCoreApplication::translate("MainWindow", "PushButton", nullptr));
        pushButton_3->setText(QCoreApplication::translate("MainWindow", "PushButton", nullptr));
        pushButton_5->setText(QCoreApplication::translate("MainWindow", "PushButton", nullptr));
        pushButton_4->setText(QCoreApplication::translate("MainWindow", "PushButton", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab), QCoreApplication::translate("MainWindow", "Tab 1", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_2), QCoreApplication::translate("MainWindow", "Tab 2", nullptr));
        pushButton_6->setText(QCoreApplication::translate("MainWindow", "PushButton", nullptr));
        pushButton_7->setText(QCoreApplication::translate("MainWindow", "PushButton", nullptr));
        pushButton_8->setText(QCoreApplication::translate("MainWindow", "PushButton", nullptr));
        pushButton_9->setText(QCoreApplication::translate("MainWindow", "PushButton", nullptr));
        pushButton_10->setText(QCoreApplication::translate("MainWindow", "PushButton", nullptr));
        pushButton_11->setText(QCoreApplication::translate("MainWindow", "PushButton", nullptr));
        tabWidget_2->setTabText(tabWidget_2->indexOf(tab_3), QCoreApplication::translate("MainWindow", "Tab 1", nullptr));
        tabWidget_2->setTabText(tabWidget_2->indexOf(tab_4), QCoreApplication::translate("MainWindow", "Tab 2", nullptr));
        menuFile->setTitle(QCoreApplication::translate("MainWindow", "File", nullptr));
        menuEdit->setTitle(QCoreApplication::translate("MainWindow", "Edit", nullptr));
        menuView->setTitle(QCoreApplication::translate("MainWindow", "View", nullptr));
        menuRefractor->setTitle(QCoreApplication::translate("MainWindow", "Refractor", nullptr));
        menuRun->setTitle(QCoreApplication::translate("MainWindow", "Run", nullptr));
        menuProjects->setTitle(QCoreApplication::translate("MainWindow", "Projects", nullptr));
        menuWindow->setTitle(QCoreApplication::translate("MainWindow", "Window", nullptr));
        toolBar->setWindowTitle(QCoreApplication::translate("MainWindow", "toolBar", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
