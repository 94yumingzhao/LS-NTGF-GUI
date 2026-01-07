// cplex_settings_widget.h - CPLEX Settings Widget

#ifndef CPLEX_SETTINGS_WIDGET_H_
#define CPLEX_SETTINGS_WIDGET_H_

#include <QWidget>

class QLineEdit;
class QSpinBox;
class QPushButton;

class CplexSettingsWidget : public QWidget {
    Q_OBJECT

public:
    explicit CplexSettingsWidget(QWidget* parent = nullptr);

    QString GetWorkDir() const;
    int GetWorkMem() const;
    int GetThreads() const;

private slots:
    void OnBrowseWorkDir();

private:
    void SetupUi();

    QLineEdit* workdir_edit_;
    QPushButton* workdir_browse_;
    QSpinBox* workmem_spin_;
    QSpinBox* threads_spin_;
};

#endif  // CPLEX_SETTINGS_WIDGET_H_
