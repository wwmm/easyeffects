from setuptools import setup

setup(name='PulseEffects',
      version='0.1',
      description='Audio effects for Pulseaudio applications',
      url='https://github.com/wwmm/PulseEffects',
      author='wwmm',
      author_email='wellington.melo@cefet-rj.br',
      license='GPL3',
      packages=['pulseeffects'],
      install_requires=[
          'gi',
          'gst-plugins-good',
          'gst-plugins-bad',
          'gst-python',
          'swh-plugins'
      ],
      scripts=['bin/pulse_effects'],
      include_package_data=True,
      zip_safe=False)
