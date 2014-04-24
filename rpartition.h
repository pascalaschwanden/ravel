#ifndef RPARTITION_H
#define RPARTITION_H

#include "event.h"
#include "general_util.h"
#include "clusterprocess.h"
#include <QList>
#include <QSet>
#include <QVector>
#include <QMap>
#include <climits>

class Gnome;

class Partition
{
public:
    Partition();
    ~Partition();
    void addEvent(Event * e);
    void deleteEvents();
    void sortEvents();
    void step();

    // Based on step
    bool operator<(const Partition &);
    bool operator>(const Partition &);
    bool operator<=(const Partition &);
    bool operator>=(const Partition &);
    bool operator==(const Partition &);

    unsigned long long int distance(Partition * other); // Time gap between partitions
    void setMergables(bool considerCollectives); // For leap merge - which children can we merge to
    void calculate_dag_leap();
    QString generate_process_string(); // For debugging
    Partition * newest_partition(); // When we're merging, find what this merged into
    int num_events();

    // Core partition information, events per process and step summary
    QMap<int, QList<Event *> *> * events;
    int max_step;
    int max_global_step;
    int min_global_step;
    int dag_leap;

    // For message merge
    bool mark;

    // For dag / Tarjan / merging
    QSet<Partition *> * parents;
    QSet<Partition *> * children;
    QSet<Partition *> * old_parents;
    QSet<Partition *> * old_children;
    QSet<Partition *> * mergable_parents;
    QSet<Partition *> * mergable_children;
    Partition * new_partition;
    int tindex;
    int lowlink;

    // For leap merge
    bool leapmark;
    QSet<Partition *> * group;

    // For graph drawing
    QString gvid;

    // For gnome and clustering
    Gnome * gnome;
    int gnome_type;
    QVector<ClusterProcess *> * cluster_processes;
    void makeClusterVectors(QString metric);
    QMap<int, QVector<long long int> *> * cluster_vectors;
    QMap<int, int> * cluster_step_starts;

private:
    // Stepping logic -- probably want to rewrite
    void step_receive(Message * msg);
    void finalize_steps();
    void restep();

    QList<Event *> * free_recvs;

};

#endif // RPARTITION_H