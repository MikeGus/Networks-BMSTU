import smtplib
import sys
import os


def code_in_file(file, code):
    for line in file:
        if code in line:
            return True

    return False


if __name__ == '__main__':
    USER = os.environ['EMAIL']
    PASSWORD = os.environ['PASSWORD']
    folder = sys.argv[1]
    code = sys.argv[2]

    files = os.listdir(folder)

    with smtplib.SMTP('smtp.yandex.ru', port=587) as s:
        s.ehlo()
        s.starttls()
        s.ehlo()
        s.login(USER, PASSWORD)
        for file in files:
            filename = os.path.join(folder, file)
            if os.path.isfile(filename):
                with open(filename) as f:
                    code_present = code_in_file(f, code)
                if code_present:
                    with open(filename) as f:
                        header = 'To:' + USER + '\n' + 'From: ' + USER + '\n' + 'Subject: Content of ' + filename + '\n'
                        msg = header + f.read()
                        s.sendmail(USER, USER, msg)
