/*
 * Copyright (C) 2015 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#include "ignition/rendering/ogre/OgreMaterial.hh"
#include "ignition/rendering/ogre/OgreConversions.hh"
#include "ignition/rendering/ogre/OgreRenderEngine.hh"
#include "ignition/rendering/ogre/OgreScene.hh"

using namespace ignition;
using namespace rendering;

//////////////////////////////////////////////////
OgreMaterial::OgreMaterial()
{
}

//////////////////////////////////////////////////
OgreMaterial::~OgreMaterial()
{
}

//////////////////////////////////////////////////
bool OgreMaterial::GetLightingEnabled() const
{
  return this->ogrePass->getLightingEnabled();
}

//////////////////////////////////////////////////
void OgreMaterial::SetLightingEnabled(bool _enabled)
{
  this->ogrePass->setLightingEnabled(_enabled);
  this->UpdateColorOperation();
}

//////////////////////////////////////////////////
gazebo::common::Color OgreMaterial::GetAmbient() const
{
  return OgreConversions::Convert(this->ogrePass->getAmbient());
}

//////////////////////////////////////////////////
void OgreMaterial::SetAmbient(const gazebo::common::Color &_color)
{
  this->ogrePass->setAmbient(OgreConversions::Convert(_color));
  this->UpdateColorOperation();
  this->UpdateTransparency();
}

//////////////////////////////////////////////////
gazebo::common::Color OgreMaterial::GetDiffuse() const
{
  return OgreConversions::Convert(this->ogrePass->getDiffuse());
}

//////////////////////////////////////////////////
void OgreMaterial::SetDiffuse(const gazebo::common::Color &_color)
{
  this->ogrePass->setDiffuse(OgreConversions::Convert(_color));
}

//////////////////////////////////////////////////
gazebo::common::Color OgreMaterial::GetSpecular() const
{
  return OgreConversions::Convert(this->ogrePass->getSpecular());
}

//////////////////////////////////////////////////
void OgreMaterial::SetSpecular(const gazebo::common::Color &_color)
{
  this->ogrePass->setSpecular(OgreConversions::Convert(_color));
}

//////////////////////////////////////////////////
gazebo::common::Color OgreMaterial::GetEmissive() const
{
  return OgreConversions::Convert(this->ogrePass->getEmissive());
}

//////////////////////////////////////////////////
void OgreMaterial::SetEmissive(const gazebo::common::Color &_color)
{
  this->ogrePass->setEmissive(OgreConversions::Convert(_color));
}

//////////////////////////////////////////////////
double OgreMaterial::GetShininess() const
{
  return this->ogrePass->getShininess();
}

//////////////////////////////////////////////////
void OgreMaterial::SetShininess(double _shininess)
{
  this->ogrePass->setShininess(_shininess);
}

//////////////////////////////////////////////////
double OgreMaterial::GetTransparency() const
{
  return this->transparency;
}

//////////////////////////////////////////////////
void OgreMaterial::SetTransparency(double _transparency)
{
  this->transparency = std::min(std::max(_transparency, 0.0), 1.0);
  this->UpdateTransparency();
}

//////////////////////////////////////////////////
double OgreMaterial::GetReflectivity() const
{
  return this->reflectivity;
}

//////////////////////////////////////////////////
void OgreMaterial::SetReflectivity(double _reflectivity)
{
  this->reflectivity = std::min(std::max(_reflectivity, 0.0), 1.0);
}

//////////////////////////////////////////////////
bool OgreMaterial::GetReceiveShadows() const
{
  return this->ogreMaterial->getReceiveShadows();
}

//////////////////////////////////////////////////
void OgreMaterial::SetReceiveShadows(bool _receiveShadows)
{
  this->ogreMaterial->setReceiveShadows(_receiveShadows);
}

//////////////////////////////////////////////////
bool OgreMaterial::HasTexture() const
{
  return !this->textureName.empty();
}

//////////////////////////////////////////////////
std::string OgreMaterial::GetTexture() const
{
  return this->textureName;
}

//////////////////////////////////////////////////
void OgreMaterial::SetTexture(const std::string &_name)
{
  if (_name.empty())
  {
    this->ClearTexture();
    return;
  }

  Ogre::TexturePtr texture = this->GetTexture(_name);

  if (!texture.isNull())
  {
    this->textureName = _name;
    this->SetTextureImpl(texture);
  }
}

//////////////////////////////////////////////////
void OgreMaterial::ClearTexture()
{
  this->textureName = "";
  this->ogreTexState->setBlank();
  this->UpdateColorOperation();
}

//////////////////////////////////////////////////
bool OgreMaterial::HasNormalMap() const
{
  return !this->textureName.empty();
}

//////////////////////////////////////////////////
std::string OgreMaterial::GetNormalMap() const
{
  return this->normalMapName;
}

//////////////////////////////////////////////////
void OgreMaterial::SetNormalMap(const std::string &_name)
{
  if (_name.empty())
  {
    this->ClearNormalMap();
    return;
  }

  // TODO: implement
  this->normalMapName = _name;
}

//////////////////////////////////////////////////
void OgreMaterial::ClearNormalMap()
{
  this->normalMapName = "";
}

//////////////////////////////////////////////////
ShaderType OgreMaterial::GetShaderType() const
{
  return this->shaderType;
}

//////////////////////////////////////////////////
void OgreMaterial::SetShaderType(ShaderType _type)
{
  this->shaderType = (ShaderUtil::IsValid(_type)) ? _type : ST_PIXEL;
}

//////////////////////////////////////////////////
Ogre::MaterialPtr OgreMaterial::GetOgreMaterial() const
{
  return this->ogreMaterial;
}

//////////////////////////////////////////////////
void OgreMaterial::LoadImage(const std::string &_name, Ogre::Image &_image)
{
  try
  {
    // TODO: improve how resource paths are handled
    OgreRenderEngine::Instance()->AddResourcePath(_name);
    _image.load(_name, this->ogreGroup);
  }
  catch (const Ogre::Exception &ex)
  {
    gzerr << "Unable to load texture image: " << _name << std::endl;
  }
}

//////////////////////////////////////////////////
void OgreMaterial::SetTextureImpl(Ogre::TexturePtr _texture)
{
  this->ogreTexState->setTexture(_texture);
  this->UpdateColorOperation();
}

//////////////////////////////////////////////////
Ogre::TexturePtr OgreMaterial::GetTexture(const std::string &_name)
{
  Ogre::TextureManager &texManager = Ogre::TextureManager::getSingleton();

  if (texManager.resourceExists(_name))
  {
    return texManager.getByName(_name);
  }

  return this->CreateTexture(_name);
}

//////////////////////////////////////////////////
Ogre::TexturePtr OgreMaterial::CreateTexture(const std::string &_name)
{
  Ogre::Image image;
  Ogre::TexturePtr texture;

  this->LoadImage(_name, image);

  if (image.getWidth() == 0)
  {
    texture.setNull();
    return texture;
  }

  texture = Ogre::TextureManager::getSingleton().createManual(_name,
      this->ogreGroup, Ogre::TEX_TYPE_2D, image.getWidth(),
      image.getHeight(), 0, Ogre::PF_X8R8G8B8);

  texture->loadImage(image);
  return texture;
}

//////////////////////////////////////////////////
void OgreMaterial::UpdateTransparency()
{
  Ogre::ColourValue ambient = this->ogrePass->getAmbient();
  double alpha = (1 - this->transparency) * ambient.a;

  if (alpha < 1)
  {
    this->ogrePass->setDepthWriteEnabled(false);
    this->ogrePass->setDepthCheckEnabled(true);
    this->ogrePass->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);

    this->ogreTexState->setAlphaOperation(Ogre::LBX_SOURCE1, Ogre::LBS_MANUAL,
        Ogre::LBS_CURRENT, alpha);
  }
  else
  {
    this->ogrePass->setDepthWriteEnabled(true);
    this->ogrePass->setDepthCheckEnabled(true);
    this->ogrePass->setSceneBlending(Ogre::SBT_REPLACE);
  }
}

//////////////////////////////////////////////////
void OgreMaterial::UpdateColorOperation()
{
  Ogre::LayerBlendOperationEx operation;
  Ogre::LayerBlendSource source1;
  Ogre::LayerBlendSource source2;
  Ogre::ColourValue color;

  bool texOff = !this->HasTexture();
  bool lightOff = !this->GetLightingEnabled();

  operation = (texOff) ? Ogre::LBX_SOURCE1 : Ogre::LBX_MODULATE;
  source1 = (texOff && lightOff) ? Ogre::LBS_MANUAL : Ogre::LBS_CURRENT;
  source2 = Ogre::LBS_TEXTURE;
  color = this->ogrePass->getAmbient();

  this->ogreTexState->setColourOperationEx(operation, source1, source2, color);
}

//////////////////////////////////////////////////
void OgreMaterial::Init()
{
  BaseMaterial::Init();
  Ogre::MaterialManager &matManager = Ogre::MaterialManager::getSingleton();
  this->ogreMaterial = matManager.create(this->name, "General");
  this->ogreTechnique = this->ogreMaterial->getTechnique(0);
  this->ogrePass = this->ogreTechnique->getPass(0);
  this->ogreTexState = this->ogrePass->createTextureUnitState();
  this->ogreGroup = Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME;
  this->ogreTexState->setBlank();
  this->Reset();

  // TODO: provide function interface
  this->ogreMaterial->setTextureAnisotropy(8);
}

//////////////////////////////////////////////////
