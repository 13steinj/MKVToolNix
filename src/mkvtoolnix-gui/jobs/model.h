#ifndef MTX_MKVTOOLNIX_GUI_JOBS_JOB_MODEL_H
#define MTX_MKVTOOLNIX_GUI_JOBS_JOB_MODEL_H

#include "common/common_pch.h"

#include "mkvtoolnix-gui/jobs/job.h"

#include <QStandardItemModel>
#include <QList>
#include <QMutex>
#include <QSet>

class QAbstractItemView;

namespace mtx { namespace gui { namespace Jobs {

class Model: public QStandardItemModel {
  Q_OBJECT;
protected:
  QHash<uint64_t, JobPtr> m_jobsById;
  QSet<Job const *> m_toBeProcessed;
  QHash<uint64_t, bool> m_toBeRemoved;
  QMutex m_mutex;

  bool m_started, m_dontStartJobsNow;

public:
  // labels << QY("Description") << QY("Type") << QY("Status") << QY("Progress") << QY("Date added") << QY("Date started") << QY("Date finished");
  static int const DescriptionColumn  = 0;
  static int const TypeColumn         = 1;
  static int const StatusColumn       = 2;
  static int const ProgressColumn     = 3;
  static int const DateAddedColumn    = 4;
  static int const DateStartedColumn  = 5;
  static int const DateFinishedColumn = 6;

  static int const RowNotFound        = -1;

public:
  Model(QObject *parent);
  virtual ~Model();

  void retranslateUi();

  QList<Job *> selectedJobs(QAbstractItemView *view) const;
  uint64_t idFromRow(int row) const;
  Job *fromId(uint64_t id) const;
  int rowFromId(uint64_t id) const;
  bool hasJobs() const;

  void removeJobsIf(std::function<bool(Job const &)> predicate);
  void add(JobPtr const &job);

  void start();
  void stop();

  void startNextAutoJob();

  void saveJobs(QSettings &settings) const;
  void loadJobs(QSettings &settings);

  virtual Qt::DropActions supportedDropActions() const;
  virtual Qt::ItemFlags flags(QModelIndex const &index) const;

signals:
  void progressChanged(unsigned int progress, unsigned int totalProgress);

public slots:
  void onStatusChanged(uint64_t id, mtx::gui::Jobs::Job::Status status);
  void onProgressChanged(uint64_t id, unsigned int progress);
  void removeScheduledJobs();

protected:
  QList<QStandardItem *> createRow(Job const &job) const;
  void setRowText(QList<QStandardItem *> const &items, Job const &job) const;
  QList<QStandardItem *> itemsForRow(QModelIndex const &idx);

  void updateProgress();
  void processAutomaticJobRemoval(uint64_t id, Job::Status status);
  void scheduleJobForRemoval(uint64_t id);
};

}}}

#endif  // MTX_MKVTOOLNIX_GUI_JOBS_JOB_MODEL_H
