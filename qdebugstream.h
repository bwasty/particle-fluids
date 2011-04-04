#ifndef QDEBUGSTREAM_H
#define QDEBUGSTREAM_H

#include <iostream>
#include <streambuf>
#include <string>

#include <QtGui/QListWidget>

        class QDebugStream : public std::basic_streambuf<char>
        {
        public:
			// TODO: QDebugStream: another parameter: stream prefix? or in Ogre log?
            QDebugStream(std::ostream &stream, QPlainTextEdit* text_edit) : m_stream(stream)
            {
                log_window = text_edit;
                m_old_buf = stream.rdbuf();
                stream.rdbuf(this);
            }
            ~QDebugStream()
            {
                // output anything that is left
                if (!m_string.empty())
                    analyzeText(m_string);

                m_stream.rdbuf(m_old_buf);
            }

        protected:
            enum MessageType {
                Information,
                Warning,
                Error,
                Blank
            };

            virtual int_type overflow(int_type v)
            {
                if (v == '\n') {
                    analyzeText(m_string);
                    m_string.erase(m_string.begin(), m_string.end());
                }
                else
                    m_string += v;

                return v;
            }

            virtual std::streamsize xsputn(const char *p, std::streamsize n)
            {
                m_string.append(p, p + n);

                int pos = 0;
                while (pos != std::string::npos) {
                    pos = m_string.find('\n');
                    if (pos != std::string::npos) {
                        std::string tmp(m_string.begin(), m_string.begin() + pos);
                        analyzeText(tmp);
                        m_string.erase(m_string.begin(), m_string.begin() + pos + 1);
                    }
                }

                return n;
            }

            virtual void analyzeText(std::string str) {
                int pos;

                //pos = str.find("blablub"); // vrs style
                //if(pos != std::string::npos) {
                //    addItem(Warning, str);
                //    return;
                //}

                addItem(Blank, str);
            }

            virtual void addItem(MessageType type, std::string str) {
                if(str.length() > 1) {
      //              switch(type) {
      //                  case Information:
				//}

                    //log_window->scrollToBottom();

                    // TODO!!!: error here - seems to be called from different thread...
					log_window->appendPlainText(QString("%1: %2").arg(QTime::currentTime().toString()).arg(QString::fromStdString(str)));
                }
            }
        	
        private:
            std::ostream &m_stream;
            std::streambuf *m_old_buf;
            std::string m_string;
            QPlainTextEdit* log_window;
        };

#endif // QDEBUGSTREAM_H