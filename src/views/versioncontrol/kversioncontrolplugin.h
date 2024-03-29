/*
 * SPDX-FileCopyrightText: 2011 Vishesh Yadav <vishesh3y@gmail.com>
 * SPDX-FileCopyrightText: 2011 Peter Penz <peter.penz19@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-only
 */

#ifndef KVERSIONCONTROLPLUGIN_H
#define KVERSIONCONTROLPLUGIN_H

#include <dolphinvcs_export.h>

#include <QAction>
#include <QObject>

class KFileItemList;
class KFileItem;
/**
 * @brief Base class for version control plugins.
 *
 * Enables the file manager to show the version state
 * of a versioned file. To write a custom plugin, the following
 * steps are required (in the example below it is assumed that a plugin for
 * Subversion will be written):
 *
 * - Create a fileviewsvnplugin.json file with the following content:
 *   <code>
 *   {
 *      "KPlugin": {
 *          "Description": "The svn plugin",
 *          "Name": "Svn"
 *      }
 *   }

 *   </code>
 *
 * - Create a class FileViewSvnPlugin derived from KVersionControlPlugin and
 *   implement all abstract interfaces (fileviewsvnplugin.h, fileviewsvnplugin.cpp).
 *
 * - Take care that the constructor has the following signature:
 *   <code>
 *   FileViewSvnPlugin(QObject* parent, const QList<QVariant>& args);
 *   </code>
 *
 * - Add the following lines at the top of fileviewsvnplugin.cpp:
 *   <code>
 *   #include <KPluginFactory>
 *   K_PLUGIN_CLASS_WITH_JSON(FileViewSvnPlugin, "fileviewsvnplugin.json")
 *   </code>
 *
 * - Add the following lines to your CMakeLists.txt file:
 *   <code>
 *   kcoreaddons_add_plugin(fileviewsvnplugin SOURCES fileviewsvnplugin.cpp INSTALL_NAMESPACE "dolphin/vcs")
 *   target_link_libraries(fileviewsvnplugin DolphinVcs)
 *   </code>
 *
 * General implementation notes:
 *
 *  - The implementations of beginRetrieval(), endRetrieval() and versionState()
 *    can contain blocking operations, as Dolphin will execute
 *    those methods in a separate thread. It is assured that
 *    all other methods are invoked in a serialized way, so that it is not necessary for
 *    the plugin to use any mutex.
 *
 * -  Dolphin keeps only one instance of the plugin, which is instantiated shortly after
 *    starting Dolphin. Take care that the constructor does no expensive and time
 *    consuming operations.
 *
 * @since 4.8
 */
class DOLPHINVCS_EXPORT KVersionControlPlugin : public QObject
{
    Q_OBJECT

public:
    enum ItemVersion {
        /** The file is not under version control. */
        UnversionedVersion,
        /**
         * The file is under version control and represents
         * the latest version.
         */
        NormalVersion,
        /**
         * The file is under version control and a newer
         * version exists on the main branch.
         */
        UpdateRequiredVersion,
        /**
         * The file is under version control and has been
         * modified locally. All modifications will be part
         * of the next commit.
         */
        LocallyModifiedVersion,
        /**
         * The file has not been under version control but
         * has been marked to get added with the next commit.
         */
        AddedVersion,
        /**
         * The file is under version control but has been marked
         * for getting removed with the next commit.
         */
        RemovedVersion,
        /**
         * The file is under version control and has been locally
         * modified. A modification has also been done on the main
         * branch.
         */
        ConflictingVersion,
        /**
         * The file is under version control and has local
         * modifications, which will not be part of the next
         * commit (or are "unstaged" in git jargon).
         * @since 4.6
         */
        LocallyModifiedUnstagedVersion,
        /**
         * The file is not under version control and is listed
         * in the ignore list of the version control system.
         * @since 4.8
         */
        IgnoredVersion,
        /**
         * The file is tracked by the version control system, but
         * is missing in the directory (e.g. by deleted without using
         * a version control command).
         * @since 4.8
         */
        MissingVersion
    };

    KVersionControlPlugin(QObject *parent = nullptr);
    ~KVersionControlPlugin() override;

    /**
     * Returns the name of the file which stores
     * the version controls information.
     * (e. g. .svn, .cvs, .git).
     */
    virtual QString fileName() const = 0;

    /**
     * Returns the path of the local repository root for the versioned directory
     * Returns an empty QString when directory is not part of a working copy
     */
    virtual QString localRepositoryRoot(const QString &directory) const;

    /**
     * Is invoked whenever the version control
     * information will get retrieved for the directory
     * \p directory. It is assured that the directory
     * contains a trailing slash.
     */
    virtual bool beginRetrieval(const QString &directory) = 0;

    /**
     * Is invoked after the version control information has been
     * received. It is assured that
     * KVersionControlPlugin::beginRetrieval() has been
     * invoked before.
     */
    virtual void endRetrieval() = 0;

    /**
     * @return The version for the item \p item.
     *         It is assured that KVersionControlPlugin::beginRetrieval() has been
     *         invoked before and that the file is part of the directory specified
     *         in beginRetrieval().
     */
    virtual ItemVersion itemVersion(const KFileItem &item) const = 0;

    /**
     * @return List of actions that are available for the \p items in a version controlled
     *         path.
     */
    virtual QList<QAction *> versionControlActions(const KFileItemList &items) const = 0;

    /**
     * @return List of actions that are available for the out of version control
     *         items \p items. It's opposed to the \p versionedActions. Common usage
     *         is for clone/checkout actions.
     * @since 21.04
     */
    virtual QList<QAction *> outOfVersionControlActions(const KFileItemList &items) const = 0;

Q_SIGNALS:
    /**
     * Should be emitted when the version state of items might have been changed
     * after the last retrieval (e. g. by executing a context menu action
     * of the version control plugin). The file manager will be triggered to
     * update the version states of the directory \p directory by invoking
     * KVersionControlPlugin::beginRetrieval(),
     * KVersionControlPlugin::itemVersion() and
     * KVersionControlPlugin::endRetrieval().
     */
    void itemVersionsChanged();

    /**
     * Is emitted if an information message with the content \a msg
     * should be shown.
     */
    void infoMessage(const QString &msg);

    /**
     * Is emitted if an error message with the content \a msg
     * should be shown.
     */
    void errorMessage(const QString &msg);

    /**
     * Is emitted if an "operation completed" message with the content \a msg
     * should be shown.
     */
    void operationCompletedMessage(const QString &msg);
};

#endif // KVERSIONCONTROLPLUGIN_H
