#pragma once

#include <QString>

namespace SystemFilePersistence
{
/**
 * Copies a live system file to the persistent lower filesystem through NX
 * Overlayroot.
 *
 * The caller must have the privileges needed to run overlayroot-chroot. The
 * source file's owner, group, and mode are preserved.
 */
bool persist(const QString &path, QString *errorMessage = nullptr);
bool persistSymlink(const QString &target, const QString &path, QString *errorMessage = nullptr);
}
