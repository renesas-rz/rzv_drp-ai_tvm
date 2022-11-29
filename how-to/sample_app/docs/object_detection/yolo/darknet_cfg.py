import re

class DarknetLayer:
    def __init__(self, name):
        self.name = name
        self.params = {}

    def __repr__(self):
        s = f"[{self.name}]\n"
        for k, v in self.params.items():
            s += f"  {k} = {v}\n"

        return s

class DarknetConfig:
    def __init__(self, path):
        self.sections = {}
        self.layers = []
        self.load(path)

    def __repr__(self):
        s = ""
        for section_name, params in self.sections.items():
            s += f"[{section_name}]\n"
            for k, v in params.items():
                s += f"  {k} = {v}\n"

            s += "\n"

        s += "===== Layer =====\n"
        for layer in self.layers:
            s += layer.__repr__()

        return s

    def load(self, path, encoding='utf-8'):
        with open(path, encoding=encoding) as f:
            self.sections = {}
            self.layers = []
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

                    if section_name in ['net']:
                        if section_name not in self.sections:
                            self.sections[section_name] = {}
                    else:
                        self.layers.append(DarknetLayer(section_name))

                m = re.match(r"(.*)=(.*)", line)
                if m:
                    name = m.group(1).strip()
                    value = m.group(2).strip()

                    if section_name in ['net']:
                        self.sections[section_name][name] = value
                    else:
                        self.layers[-1].params[name] = value

                line = f.readline()
