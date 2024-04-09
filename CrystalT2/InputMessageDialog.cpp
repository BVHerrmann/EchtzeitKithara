#include "InputMessageDialog.h"

InputMessageDialog::InputMessageDialog(QWidget* parent) : QDialog(parent)
{
    ui.setupUi(this);

    m_PopupInputMessageDialog = (PopupInputMessageDialog*)(parent);
    connect(ui.pushButtonYes, &QPushButton::clicked, this, &InputMessageDialog::SlotYes);
    connect(ui.pushButtonNo, &QPushButton::clicked, this, &InputMessageDialog::SlotNo);
}

InputMessageDialog::~InputMessageDialog()
{
}

void InputMessageDialog::SlotCloseDialog()
{
    m_PopupInputMessageDialog->accept();
}

void InputMessageDialog::SlotYes()
{
    // emit SignalYes();
    m_PopupInputMessageDialog->SendSignalYes();
    m_PopupInputMessageDialog->accept();
}

void InputMessageDialog::SlotNo()
{
    // emit SignalNo();
    m_PopupInputMessageDialog->SendSignalNO();
    m_PopupInputMessageDialog->reject();
}

QString InputMessageDialog::GetInputString()
{
    return ui.lineEditInputData->text();
}

int InputMessageDialog::OpenYesNoDialog(bool Modal)
{
    ui.lineEditInputData->hide();
    ui.pushButtonYes->show();
    ui.pushButtonNo->show();
    if (!Modal)
        m_PopupInputMessageDialog->show();
    else
        return m_PopupInputMessageDialog->exec();

    return 0;
}

int InputMessageDialog::OpenYesNoDialog(const QString& LineEditMessage, bool Modal)
{
    ui.pushButtonYes->show();
    ui.pushButtonNo->show();

    if (LineEditMessage.isEmpty())
        ui.lineEditInputData->hide();
    else {
        ui.lineEditInputData->setText(LineEditMessage);
        ui.lineEditInputData->show();
    }
    if (!Modal)
        m_PopupInputMessageDialog->show();
    else
        return m_PopupInputMessageDialog->exec();

    return 0;
}

void InputMessageDialog::ClearInputString()
{
    ui.lineEditInputData->setText("");
}
