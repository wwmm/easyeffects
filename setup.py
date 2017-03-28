from setuptools import setup, find_packages

setup(name='PulseEffects',
      version='1.0.5',
      description='Audio effects for Pulseaudio applications',
      url='https://github.com/wwmm/PulseEffects',
      author='wwmm',
      author_email='wellington.melo@cefet-rj.br',
      license='GPL3',
      packages=find_packages(),
      install_requires=[
          'gi',
          'gst-plugins-bad',
          'gst-python',
          'swh-plugins',
          'caps'
      ],
      scripts=['pulseeffects'],
      include_package_data=True,
      zip_safe=False)
