import xml.etree.ElementTree as ET
import hashlib
import time
from Crypto.Random import random
from datetime import datetime

def utcstamp():
	return tstamp(datetime.utcnow())

def tstamp(t):
	return str.format("{:04}{:02}{:02}{:02}{:02}{:02}",
					  t.year,t.month,t.day,t.hour,t.minute,t.second)

class KMFontList:

	def __init__(self,path,rename_rule=None):
		tree = ET.parse(path)
		root = tree.getroot()
		if root.tag not in ["fontlist"]:
			raise Exception("Root element must be <fontlist> (in " + path + ")")
		groups = []
		
		if rename_rule is None:
			self.rename_rule = {}
		else:
			self.rename_rule = rename_rule
		
		self.internal_files = set()
		self.external_files = set()
		self.opt_external_files = set()
		self.path = path
		
		if "version" in root.attrib:
			self.version = root.attrib["version"]
		else:
			self.version = utcstamp()
			root.attrib["version"] = self.version

		if "id" in root.attrib:
			self.id = root.attrib["id"]
		else:
			self.id = "com.nexstreaming.kinemaster.fontlist"
		
		for child in root:
			if child.tag=="group":
				labels = {}
				fonts = {}
				group = {"labels":labels,"fonts":fonts,"id":child.attrib["id"]}
				groups.append(group)
				for gchild in child:
					if gchild.tag=="string":
						labels[gchild.attrib["locale"]] = gchild.text
					elif gchild.tag=="font":
						file = gchild.attrib["file"]
						sample = gchild.attrib["sample"]
						name = gchild.attrib["name"]
						
						if file is not None:
							if file in self.rename_rule:
								file = self.rename_rule[file]
								gchild.attrib["file"] = file
							self.external_files.add(file)

						if sample is not None:
							if sample in self.rename_rule:
								sample = self.rename_rule[sample]
								gchild.attrib["sample"] = sample
							self.opt_external_files.add(sample)
						
						fonts[gchild.attrib["id"]] = {
							"file":file,
							"sample":sample,
							"name":name
						}
		self.groups = groups

	@property
	def summary(self):
		result = {
				"id":self.id,
				"version":self.version,
				"groups":self.groups,
				"package_type":"fontlist",
				"package_date":datetime.utcnow().toordinal()
				}
		return result


class KEDLFile:

	def __init__(self,path,rename_rule=None):
		tree = ET.parse(path)
		root = tree.getroot()
		if root.tag not in ["theme","effect"]:
			raise Exception("Root element must be <theme> or <effect> (in " + path + ")")
		self.path = path
		self.type = root.tag
		self.id = root.attrib["id"]
		if rename_rule is None:
			self.rename_rule = {}
		else:
			self.rename_rule = rename_rule
		self.strings = {}
		self._initkwmap()
		for s in root.findall("string"):
			n = s.attrib["name"]
			l = s.attrib["locale"]
			if n not in self.strings:
				self.strings[n] = {}
			self.strings[n][l] = s.text
		if "version" in root.attrib:
			self.version = root.attrib["version"]
		else:
			self.version = utcstamp()
			root.attrib["version"] = self.version
		self.internal_files = set()
		self.external_files = set()
		self.opt_external_files = set()
		self.icon = root.attrib.get("icon")
		if self.icon is not None:
			if self.icon in self.rename_rule:
				self.icon = self.rename_rule[self.icon]
				root.attrib["icon"] = self.icon
			self.external_files.add(self.icon)
		self.preview = root.attrib.get("preview")
		if self.preview is not None:
			if self.preview in self.rename_rule:
				self.preview = self.rename_rule[self.preview]
				root.attrib["preview"] = self.preview
			self.opt_external_files.add(self.preview)
		if self.type=="theme":
			self._parsetheme(root)
		else:
			self._parseeffect(root)
		for e in root.iter():
			if e.tag=="auth" and "match" in e.attrib and "value" in e.attrib:
				if e.attrib["match"]=="device":
					dev = e.attrib["value"]
					salt = hex(random.getrandbits(64))[2:-1]
					#salt = hex(os.urandom(64//8))[2:-1]
					hash = hashlib.new('sha1',salt + dev).hexdigest()
					e.attrib["value"] = salt + ":" + hash
#		for e in root.iter():
#			if e.tail is not None and len(e.tail.strip()) < 1:
#				e.tail = ""
#			if e.text is not None and len(e.text.strip()) < 1:
#				e.text = ""
		for e in root.iter():
			if e.tail is not None and len(e.tail.strip()) < 1:
				e.tail = e.tail.strip(" \t")
			if e.text is not None and len(e.text.strip()) < 1:
				e.text = e.text.strip(" \t")
		self.xml = ET.tostring(root,encoding="utf-8")
		for e in root.iter():
			if e.tag in self._kwmap:
				e.tag = self._kwmap[e.tag]
#			if e.tail is not None and len(e.tail.strip()) < 1:
#				e.tail = ""
#			if e.text is not None and len(e.text.strip()) < 1:
#				e.text = ""
			for a in e.attrib:
				if a in self._kwmap:
					v = e.attrib[a]
					del e.attrib[a]
					if v in self._kwmap:
						v = "!$" + self._kwmap[v]
					e.attrib[self._kwmap[a]] = v
				else:
					v = e.attrib[a]
					if v in self._kwmap:
						v = "!$" + self._kwmap[v]
						e.attrib[a] = v
		self.minixml = ET.tostring(root,encoding="utf-8")

	@property
	def summary(self):
		result = {
				"id":self.id,
				"strings":self.strings,
				"version":self.version,
				"thumbnail":self.icon,
				"preview":self.preview,
				"package_type":"kedl",
				"package_date":datetime.utcnow().toordinal()
				}
		return result
			
	def __str__(self):
		return( "<KEDLFile (" + self.type + ") " + self.id + ">" )

	def _parsetheme(self,root):
		self.ref = {}
		cond_idx = 0
		for ref in root.findall("ref"):
			if "type" in ref.attrib:
				t = ref.attrib["type"]
				if t=="custom":
					self.ref["custom" + str(cond_idx)] = ref.text
					cond_idx += 1
				else:
					self.ref[t] = ref.text
			else:
				self.ref["custom" + str(cond_idx)] = ref.text
				cond_idx += 1
		self.music = root.attrib.get("music")
		if self.music is not None:
			if self.music in self.rename_rule:
				self.music = self.rename_rule[self.music]
				root.attrib["music"] = self.music
			self.external_files.add(self.music)

	def _parseeffect(self,root):
		if "theme" in root.attrib and root.attrib["theme"].lower()=="true":
			self.themeonly = True
		else:
			self.themeonly = False
		# Texture source files (images)
		for t in root.findall("texture"):
			if "src" in t.attrib:
				a = t.attrib["src"]
				if a in self.rename_rule:
					a = self.rename_rule[a]
					t.attrib["src"] = a
			if "typeface" in t.attrib:
				a = t.attrib["typeface"]
				if a in self.rename_rule:
					a = self.rename_rule[a]
					t.attrib["typeface"] = a

		self.internal_files.update(
			[t.attrib["src"] for t in
			 root.findall("texture") if "src" in t.attrib])

		self.external_files.update(
			[t.attrib["typeface"] for t in
			 root.findall("texture") if
			 "typeface" in t.attrib
			 and ":" not in t.attrib.get("typeface")])

	def validate(self,items):
		pass

	def _initkwmap(self):
		self._kwmap = {
			"effect":			"ee",
			"string":			"xs",
			"allow":			"va",
			"deny":				"vd",
			"name":				"vqn",
			"desc":				"vqd",
			"opening":			"vqo",
			"middle":			"vqm",
			"ending":			"vqe",
			"accent":			"vqa",
			"transition":		"vqt",
			"title":			"vql",
			"text":				"vqf",
			"video1":			"vqb",
			"video2":			"vqc",
			"perspective":		"vqv",
			"device":			"vqg",
			"all":				"fz",
			"auth":				"xa",
			"match":			"xm",
			"ref":				"xr",
			"theme":			"tt",
			"texture":			"xx",
			"userfield":		"uu",
			"fieldlabel":		"ll",
			"animatedvalue":	"aa",
			"part":				"q",
			"keyframe":			"k",
			"projection":		"rr",
			"translate":		"nn",
			"rect":				"cc",
			"style":			"ss",
			"trianglestrip":	"ii",
			"alpha":			"hh",
			"bounds":			"bb",
			"color":			"oo",
			"width":			"ww",
			"height":			"gg",
			"id":				"dd",
			"src":				"jj",
			"video":			"vv",
			"version":			"vz",
			"music":			"mz",
			"icon":				"ic",
			"locale":			"lc",
			"value":			"f",
			"scale":			"zz",
			"offset":			"yy",
			"type":				"qq",
			"uniform":			"mm",
			"timingfunction":	"tf",
			"mask":				"mk",
			"animframe":		"af",
			"textmargin":		"ttm",
			"textalign":		"ttl",
			"textsize":			"tts",
			"typeface":			"ttf",
			"autosize":			"tta",
			"fill":				"fl",
			"fillcolor":		"fc",
			"intime":			"ta",
			"intimefirst":		"tb",
			"outtime":			"tc",
			"polygon":			"gn",
			"outtimelast":		"td",
			"rotate":			"rz",
			"angle":			"ra",
			"axis":				"rx",
			"visibility":		"vx",
			"rendertest":		"ur",
			"check":			"ck",
			"aspect":			"px",
			"dest":				"dt",
			"near":				"nr",
			"far":				"fr"
			}
