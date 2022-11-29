import re

class CfgParameter:
    def __init__(self, name):
        self.name = name
        self.params = {}

class IniFile:
    def __init__(self, path):
        self.sections = {}
        self.architecture = []
        self.model_dict = {}
        self.load(path)

    def load(self, path, encoding='utf-8'):
        with open(path, encoding=encoding) as f:
            self.sections = {}
            self.architecture = []
            section_name = None

            line = f.readline()
            while line:
                line = line.strip().lower()

                # parse
                m = re.match(r"#", line)
                if m:
                    line = f.readline()
                    continue

                m = re.match(r"\[(.*)\]", line)
                if m:
                    section_name = m.group(1).strip()
                    self.architecture.append(CfgParameter(section_name))

                m = re.match(r"(.*)=(.*)", line)
                if m:
                    name = m.group(1).strip()
                    value = m.group(2).strip()
                    if re.match(r"\[(.*)\]", value):
                        list = value.strip(r"\[\]")
                        list = list.split(",")
                        inout_list = []
                        for value in list:
                            value=value.strip(" ")
                            inout_list.append(value.strip(r"\"\'\n　\t"))
                        self.architecture[-1].params[name] = inout_list
                    else:
                        self.architecture[-1].params[name] = value
                line = f.readline()

        model_dict = {}
        cnt = 0
        for model in self.architecture :
            model_dict[model.name] = cnt
            cnt += 1
        self.model_dict = model_dict
