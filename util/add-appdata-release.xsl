<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output method="xml" version="1.0" encoding="UTF-8" indent="yes"/>
  <xsl:strip-space elements="*"/>

  <xsl:param name="version"/>
  <xsl:param name="date"/>

  <xsl:template match="*" priority="-1">
    <xsl:element name="{name()}">
      <xsl:apply-templates select="node()|@*"/>
    </xsl:element>
  </xsl:template>

  <xsl:template match="node()|@*" priority="-2">
    <xsl:copy/>
  </xsl:template>

  <xsl:template match="releases">
    <releases>
      <release>
        <xsl:attribute name="version"><xsl:value-of select="$version"/></xsl:attribute>
        <xsl:attribute name="date"><xsl:value-of select="$date"/></xsl:attribute>
      </release>
      <xsl:apply-templates select="node()|@*"/>
    </releases>
  </xsl:template>
</xsl:stylesheet>
