/*
 * Copyright (c) 2011-2014 BlackBerry Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "applicationui.hpp"

#include <bb/cascades/Application>
#include <bb/cascades/QmlDocument>
#include <bb/cascades/AbstractPane>
#include <bb/cascades/LocaleHandler>
#include <bb/system/Clipboard>
#include <bb/device/DisplayInfo>

using namespace bb::cascades;
ApplicationUI::ApplicationUI() :
        QObject()
{
    // prepare the localization
    m_pTranslator = new QTranslator(this);
    m_pLocaleHandler = new LocaleHandler(this);

    bool res = QObject::connect(m_pLocaleHandler, SIGNAL(systemLanguageChanged()), this,
            SLOT(onSystemLanguageChanged()));
    // This is only available in Debug builds
    Q_ASSERT(res);
    // Since the variable is not used in the app, this is added to avoid a
    // compiler warning
    Q_UNUSED(res);

    // initial load
    onSystemLanguageChanged();

    // Create scene document from main.qml asset, the parent is set
    // to ensure the document gets destroyed properly at shut down.
    QmlDocument *qml = QmlDocument::create("asset:///main.qml").parent(this);
    qml->setContextProperty("_app", this);

    bb::device::DisplayInfo display;
    int width = display.pixelSize().width();
    int height = display.pixelSize().height();

    QDeclarativePropertyMap* displayProperties = new QDeclarativePropertyMap;
    displayProperties->insert("width", QVariant(width));
    displayProperties->insert("height", QVariant(height));

    qml->setContextProperty("DisplayInfo", displayProperties);

    // Create root object for the UI
    AbstractPane *root = qml->createRootObject<AbstractPane>();

    // Set created root object as the application scene
    Application::instance()->setScene(root);
}

void ApplicationUI::onSystemLanguageChanged()
{
    QCoreApplication::instance()->removeTranslator(m_pTranslator);
    // Initiate, load and install the application translation files.
    QString locale_string = QLocale().name();
    QString file_name = QString("DevToolsPro_%1").arg(locale_string);
    if (m_pTranslator->load(file_name, "app/native/qm")) {
        QCoreApplication::instance()->installTranslator(m_pTranslator);
    }
}

void ApplicationUI::setValue(QString field, QString input)
{
    AppSettings::saveValueFor(field, input);
}

QString ApplicationUI::getValue(QString input, QString def)
{
    QString result = AppSettings::getValueFor(input, def);
    return result;
}

QString ApplicationUI::readTextFile(QString filepath)
{
    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return "";
    QString c = "";
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        c.append(line).append("\r\n");
    }
    return c;
}

bool ApplicationUI::writeTextFile(QString filepath, QString filecontent)
{
    QFile textfile(filepath);
    if (textfile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&textfile);
        out << filecontent;
        textfile.close();
        return (true);
    } else {
        return (false);
    }

}

QString ApplicationUI::getTextFromClipboard()
{
    bb::system::Clipboard cp;
    QByteArray data = cp.value("text/plain");
    if (!data.isEmpty()) {
        return QString::fromAscii(data, data.size());
    } else {
        return "";
    }
}

void ApplicationUI::setTextToClipboard(QString t)
{
    bb::system::Clipboard cp;
    cp.clear();
    cp.insert("text/plain", t.toAscii());
}
