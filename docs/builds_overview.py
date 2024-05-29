#!/usr/bin/env python

# builds_overview.py
#
#############################################################################################################
# This script parses all documentation substitution files to determine in what builds a plugin is available
# Collection A..G, Display, Energy and Neopixel, IR and IRext get Normal plugins injected
# Collection plugins are also injected into Collection A..G
# All plugins get injected into MAX build set
# Some build sets have exceptions for plugins not available
# The output generation order is determined by how they are ordered in list 'buildColors'
# When adding or removing a build set, this script may need adjustments!

# Changelog:
# 2024-05-04 tonhuisman: Working and documented
# 2024-04-28 tonhuisman: Initial script

import os
import re
import json

# Must be a subfolder of source, as the included filenames have a ../ prefix!
basePath = "source/_templates/"

# Gather data
allBuilds = {}

# Not mentioned as a build in documentation, implicit
appendBuilds = {'MAX'}

# What build set to add plugins also
appendAlso = {
  'NORMAL': {'CLIMATE', 'COLLECTION A', 'COLLECTION B', 'COLLECTION C', 'COLLECTION D', 'COLLECTION E', 'COLLECTION F', 'COLLECTION G', 'DISPLAY', 'ENERGY', 'IR', 'IRext', 'NEOPIXEL'},
  'COLLECTION': {'COLLECTION A', 'COLLECTION B', 'COLLECTION C', 'COLLECTION D', 'COLLECTION E', 'COLLECTION F', 'COLLECTION G'}
  }

# Ignore these, not real build sets
excludeBuilds = {'DEVELOPMENT', 'RETIRED'}

# Plugins not included
excludePlugins = {
  'CLIMATE': {'P007', 'P008', 'P009', 'P017', 'P022', 'P027', 'P030', 'P035', 'P040', 'P041', 'P042', 'P045'},
  'DISPLAY': {'P070'},
  'MAX': {'P089'},
  # 'NEOPIXEL': {''},
  'NORMAL': {'P016', 'P035'},
}

# This list determines the order and color of the build sets to include in the generated output
buildColors = {
  'NORMAL': 'green',
  'COLLECTION A': 'yellow',
  'COLLECTION B': 'yellow',
  'COLLECTION C': 'yellow',
  'COLLECTION D': 'yellow',
  'COLLECTION E': 'yellow',
  'COLLECTION F': 'yellow',
  'COLLECTION G': 'yellow',
  'CLIMATE': 'yellow',
  'DISPLAY': 'yellow',
  'ENERGY': 'yellow',
  'IR': 'yellow',
  'IRext': 'yellow',
  'NEOPIXEL': 'yellow',
  'MAX': 'yellow',
}

# Add/update a single plugin in the list
def addOnePlugin(build, plugin, pluginName):
  if not build in allBuilds:
    allBuilds[build] = {}
  allBuilds[build].update({plugin: pluginName})

# Add a plugin to all builds it should go in
def addToAllBuilds(plugin, pluginName, builds:dict):
  for b in appendBuilds:
    if not b in builds:
      builds += {b}
  for b in builds:
    if b:
      includeIt = True
      # builds to ignore
      if b in excludeBuilds:
        includeIt = False
      # plugins per build to ignore
      if includeIt and b in excludePlugins:
        if plugin in excludePlugins[b]:
          includeIt = False
      if includeIt:
        addOnePlugin(b, plugin, pluginName)
        # Add in other builds too?
        if b in appendAlso:
          for n in appendAlso[b]:
            if includeIt and n in excludePlugins:
              if plugin in excludePlugins[n]:
                includeIt = False
            # Except when not to be included
            if includeIt:
              addOnePlugin(n, plugin, pluginName)

# Parse a single substitution file
def parseSingleSubstitutionFile(fileName):
  filepath = os.path.relpath(os.path.join(basePath, fileName), '.')
  # print(filepath) # For debugging
  pfile = open(filepath, "r")
  # Start empty
  plugin = ""
  pluginName = ""
  builds = []
  while True:
    line = pfile.readline()
    if not line:
      break
    # Parse into label, plugin ID, description and up to 4 separate builds (current max.),
    # append "(?:[^`]+`([^`]+)`)?" to regex for an extra build, if needed
    m = re.search(r"[^|]\|([PCN](\d{3}))([^\|]+)\|[^`]+`([^`]+)`(?:[^`]+`([^`]+)`)?(?:[^`]+`([^`]+)`)?(?:[^`]+`([^`]+)`)?", line)
    if m:
      if m.group(3) == "_typename": # the typename substitution should be before _status...
        if plugin != "" and plugin != m.group(1): # Changed plugin ID, store current
          addToAllBuilds(plugin, pluginName, builds)
        plugin = m.group(1)
        pluginName = m.group(4)        

      if m.group(3) == "_status":
        builds = [m.group(4), m.group(5), m.group(6), m.group(7)]
  pfile.close()
  if plugin != "": # Store last one too
    addToAllBuilds(plugin, pluginName, builds)
    
# Parse all .. include :: files
def parseSubstitutionFiles(rootFile):
  rfile = open(basePath + rootFile, "r")
  while True:
    line = rfile.readline()
    if not line:
      break
    m = re.search(r"[^:]+::(.*)", line)
    fn = m.group(1).strip()
    if fn and fn != "":
      parseSingleSubstitutionFile(fn)
  rfile.close()

# Sort Plugins on top, anything else below that
def sortPluginsBeforeControllers(pluginid):
  if pluginid[0] != 'P':
    return pluginid.lower()[0] # Lowercase sorts after uppercase, so P goes first
  return pluginid[0]

# Generate the output
def generateBuildOverview(fileName):
  filepath = os.path.relpath(os.path.join(basePath, fileName), '.')
  
  print('Writing build sets overview to:', filepath)

  output = open(filepath, "w")
  output.write('Plugins per build set\n')
  output.write('=====================\n')
  output.write('\n')
  for b in buildColors:
    if b in allBuilds:
      output.write('Build set:  :' + buildColors[b] + ':`' + b + '`\n')
      output.write('---------------------------------------------\n')
      output.write('\n')
      output.write('.. collapse:: Details...\n')
      output.write('\n')
      output.write('   .. csv-table::\n')
      output.write('      :header: "Plugin name", "Plugin number"\n')
      output.write('      :widths: 10, 5\n')
      output.write('\n')
      for p in sorted(allBuilds[b], key=sortPluginsBeforeControllers):
        output.write('      ":ref:`' + p + '_page`","' + p + '"\n')
      output.write('\n')
  output.close()

# Main entrypoint
print('Parsing substitutions for build sets...')
# Parse all Plugin substitutions
parseSubstitutionFiles('../Plugin/_plugin_substitutions.repl')
# Parse all Controller substitutions
parseSingleSubstitutionFile('../Controller/_controller_substitutions.repl')

# Generate output
generateBuildOverview('../Plugin/_plugin_sets_overview.repl')

# print(json.dumps(allBuilds,indent=2,sort_keys=True)) # For debugging
