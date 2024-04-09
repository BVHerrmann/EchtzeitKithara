#pragma once
#include <optionpanel.h>

#include <QtWidgets>

class MainAppCrystalT2;
class SelectTriggerPosOptionPanel : public OptionPanel
{
    Q_OBJECT
  public:
    SelectTriggerPosOptionPanel(MainAppCrystalT2* pMainAppCrystalT2 /*, QWidget *pParent*/);
    MainAppCrystalT2* GetMainAppCrystalT2() { return m_MainAppCrystalT2; }
    void ShowRow(int row, bool sh);
    void SetAuditTrailProperties();
    void SetRequiredAccessLevel();

  private slots:
    void SlotSetCheckedRecordImages(int);

  private:
    MainAppCrystalT2* m_MainAppCrystalT2;
    QCheckBox* m_CheckBoxRecordImages;
    QLabel* m_TextRecordImages;
    QFormLayout* m_FormLayout;
};
